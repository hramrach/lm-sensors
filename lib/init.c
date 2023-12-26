/*
    init.c - Part of libsensors, a Linux library for reading sensor data.
    Copyright (c) 1998, 1999  Frodo Looijaard <frodol@dds.nl>
    Copyright (C) 2007-2013   Jean Delvare <jdelvare@suse.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
    MA 02110-1301 USA.
*/

/* Needed for scandir() and alphasort() */
#define _BSD_SOURCE 1 /* for glibc < 2.19 */
#define _DEFAULT_SOURCE 1 /* for glibc >= 2.19 */

#include <sys/types.h>
#include <sys/stat.h>
#include <locale.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <unistd.h>
#include "sensors.h"
#include "data.h"
#include "error.h"
#include "access.h"
#include "conf.h"
#include "sysfs.h"
#include "scanner.h"
#include "init.h"

#define DEFAULT_CONFIG_FILE	ETCDIR "/sensors3.conf"
#define ALT_CONFIG_FILE		ETCDIR "/sensors.conf"
#define DEFAULT_CONFIG_DIR	ETCDIR "/sensors.d"

static void free_bus(sensors_bus *bus)
{
	free(bus->adapter);
}

static void free_config_busses(sensors_config *config)
{
	int i;

	for (i = 0; i < config->busses_count; i++)
		free_bus(&config->busses[i]);
	free(config->busses);
	config->busses = NULL;
	config->busses_count = config->busses_max = 0;
}

static int parse_config(sensors_config *config, FILE *input, const char *name)
{
	int err;
	char *name_copy;
	yyscan_t yyscanner;
	sensors_chip *current_chip = NULL;

	if (name) {
		/* Record configuration file name for error reporting */
		name_copy = strdup(name);
		if (!name_copy)
			sensors_fatal_error(__func__, "Out of memory");
		sensors_add_config_files(&name_copy);
	} else
		name_copy = NULL;

	if (sensors_scanner_init(input, name_copy, &yyscanner)) {
		err = -SENSORS_ERR_PARSE;
		goto exit_cleanup;
	}
	err = sensors_yyparse(config, current_chip, yyscanner);
	sensors_scanner_exit(yyscanner);
	if (err) {
		err = -SENSORS_ERR_PARSE;
		goto exit_cleanup;
	}

	err = sensors_substitute_busses(config);

exit_cleanup:
	free_config_busses(config);
	return err;
}

static int config_file_filter(const struct dirent *entry)
{
	return entry->d_name[0] != '.';		/* Skip hidden files */
}

static int add_config_from_dir(sensors_config *config, const char *dir)
{
	int count, res, i;
	struct dirent **namelist;

	count = scandir(dir, &namelist, config_file_filter, alphasort);
	if (count < 0) {
		/* Do not return an error if directory does not exist */
		if (errno == ENOENT)
			return 0;
		
		sensors_parse_error_wfn(strerror(errno), NULL, 0);
		return -SENSORS_ERR_PARSE;
	}

	for (res = 0, i = 0; !res && i < count; i++) {
		int len;
		char path[PATH_MAX];
		FILE *input;
		struct stat st;

		len = snprintf(path, sizeof(path), "%s/%s", dir,
			       namelist[i]->d_name);
		if (len < 0 || len >= (int)sizeof(path)) {
			res = -SENSORS_ERR_PARSE;
			continue;
		}

		/* Only accept regular files */
		if (stat(path, &st) < 0 || !S_ISREG(st.st_mode))
			continue;

		input = fopen(path, "r");
		if (input) {
			res = parse_config(config, input, path);
			fclose(input);
		} else {
			res = -SENSORS_ERR_PARSE;
			sensors_parse_error_wfn(strerror(errno), path, 0);
		}
	}

	/* Free memory allocated by scandir() */
	for (i = 0; i < count; i++)
		free(namelist[i]);
	free(namelist);

	return res;
}

/* Ideally, initialization and configuraton file loading should be exposed
   separately, to make it possible to load several configuration files. */
static int __sensors_init(sensors_config *config, FILE *input);
int sensors_init(FILE *input)
{
	return __sensors_init(&global_sensors_config, input);
}
sensors_config *sensors_init_r(FILE *input, int *err)
{
	int res = 0;
	sensors_config *config = calloc(1, sizeof(sensors_config));
	if (config) {
		res = __sensors_init(config, input);
		if (res) {
			free(config);
			config = NULL;
		}
	} else {
		res = -SENSORS_ERR_PARSE;
	}
	if (err)
		*err = res;
	return config;
}
static void __sensors_cleanup(sensors_config *config);
int __sensors_init(sensors_config *config, FILE *input)
{
	int res;

	if (!sensors_init_sysfs(config))
		return -SENSORS_ERR_KERNEL;
	if ((res = sensors_read_sysfs_bus(config)) ||
	    (res = sensors_read_sysfs_chips(config)))
		goto exit_cleanup;

	if (input) {
		res = parse_config(config, input, NULL);
		if (res)
			goto exit_cleanup;
	} else {
		const char* name;

		/* No configuration provided, use default */
		input = fopen(name = DEFAULT_CONFIG_FILE, "r");
		if (!input && errno == ENOENT)
			input = fopen(name = ALT_CONFIG_FILE, "r");
		if (input) {
			res = parse_config(config, input, name);
			fclose(input);
			if (res)
				goto exit_cleanup;

		} else if (errno != ENOENT) {
			sensors_parse_error_wfn(strerror(errno), name, 0);
			res = -SENSORS_ERR_PARSE;
			goto exit_cleanup;
		}

		/* Also check for files in default directory */
		res = add_config_from_dir(config, DEFAULT_CONFIG_DIR);
		if (res)
			goto exit_cleanup;
	}

	return 0;

exit_cleanup:
	__sensors_cleanup(config);
	return res;
}

static void free_chip_name(sensors_chip_name *name)
{
	free(name->prefix);
	free(name->path);
}

static void free_chip_features(sensors_chip_features *features)
{
	int i;

	for (i = 0; i < features->subfeature_count; i++)
		free(features->subfeature[i].name);
	free(features->subfeature);
	for (i = 0; i < features->feature_count; i++)
		free(features->feature[i].name);
	free(features->feature);
}

static void free_label(sensors_label *label)
{
	free(label->name);
	free(label->value);
}

void sensors_free_expr(sensors_expr *expr)
{
	if (expr->kind == sensors_kind_var)
		free(expr->data.var);
	else if (expr->kind == sensors_kind_sub) {
		if (expr->data.subexpr.sub1)
			sensors_free_expr(expr->data.subexpr.sub1);
		if (expr->data.subexpr.sub2)
			sensors_free_expr(expr->data.subexpr.sub2);
	}
	free(expr);
}

static void free_set(sensors_set *set)
{
	free(set->name);
	sensors_free_expr(set->value);
}

static void free_compute(sensors_compute *compute)
{
	free(compute->name);
	sensors_free_expr(compute->from_proc);
	sensors_free_expr(compute->to_proc);
}

static void free_ignore(sensors_ignore *ignore)
{
	free(ignore->name);
}

static void free_chip(sensors_chip *chip)
{
	int i;

	for (i = 0; i < chip->chips.fits_count; i++)
		free_chip_name(&chip->chips.fits[i]);
	free(chip->chips.fits);
	chip->chips.fits_count = chip->chips.fits_max = 0;

	for (i = 0; i < chip->labels_count; i++)
		free_label(&chip->labels[i]);
	free(chip->labels);
	chip->labels_count = chip->labels_max = 0;

	for (i = 0; i < chip->sets_count; i++)
		free_set(&chip->sets[i]);
	free(chip->sets);
	chip->sets_count = chip->sets_max = 0;

	for (i = 0; i < chip->computes_count; i++)
		free_compute(&chip->computes[i]);
	free(chip->computes);
	chip->computes_count = chip->computes_max = 0;

	for (i = 0; i < chip->ignores_count; i++)
		free_ignore(&chip->ignores[i]);
	free(chip->ignores);
	chip->ignores_count = chip->ignores_max = 0;
}

static void __sensors_cleanup(sensors_config *config);
void sensors_cleanup(void)
{
	__sensors_cleanup(&global_sensors_config);
}
void sensors_cleanup_r(sensors_config *config){
	if (config) {
		__sensors_cleanup(config);
		free(config);
	}
}
static void __sensors_cleanup(sensors_config *config)
{
	int i;

	for (i = 0; i < config->features_count; i++) {
		free_chip_name(&config->features[i].chip);
		free_chip_features(&config->features[i]);
	}
	free(config->features);
	config->features = NULL;
	config->features_count = config->features_max = 0;

	for (i = 0; i < config->chips_count; i++)
		free_chip(&config->chips[i]);
	free(config->chips);
	config->chips = NULL;
	config->chips_count = config->chips_max = 0;
	config->chips_subst = 0;

	for (i = 0; i < config->bus_count; i++)
		free_bus(&config->bus[i]);
	free(config->bus);
	config->bus = NULL;
	config->bus_count = config->bus_max = 0;

	for (i = 0; i < config->files_count; i++)
		free(config->files[i]);
	free(config->files);
	config->files = NULL;
	config->files_count = config->files_max = 0;
}
