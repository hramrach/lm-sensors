/*
    data.h - Part of libsensors, a Linux library for reading sensor data.
    Copyright (c) 1998, 1999  Frodo Looijaard <frodol@dds.nl>
    Copyright (C) 2007-2010   Jean Delvare <jdelvare@suse.de>

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

#ifndef LIB_SENSORS_DATA_H
#define LIB_SENSORS_DATA_H

#include "sensors.h"
#include "general.h"

/* This header file contains all kinds of data structures which are used
   for the representation of the config file data and the sensors
   data. */

/* Kinds of expression operators recognized */
typedef enum sensors_operation {
	sensors_add, sensors_sub, sensors_multiply, sensors_divide,
	sensors_negate, sensors_exp, sensors_log,
} sensors_operation;

/* An expression can have several forms */
typedef enum sensors_expr_kind {
	sensors_kind_val, sensors_kind_source, sensors_kind_var,
	sensors_kind_sub
} sensors_expr_kind;

/* An expression. It is either a floating point value, a variable name,
   an operation on subexpressions, or the special value 'sub' } */
struct sensors_expr;

typedef struct sensors_subexpr {
	sensors_operation op;
	struct sensors_expr *sub1;
	struct sensors_expr *sub2;
} sensors_subexpr;

typedef struct sensors_expr {
	sensors_expr_kind kind;
	union {
		double val;
		char *var;
		sensors_subexpr subexpr;
	} data;
} sensors_expr;

/* Config file line reference */
typedef struct sensors_config_line {
	const char *filename;
	int lineno;
} sensors_config_line;

/* Config file label declaration: a feature name, combined with the label
   value */
typedef struct sensors_label {
	char *name;
	char *value;
	sensors_config_line line;
} sensors_label;

/* Config file set declaration: a subfeature name, combined with an
   expression */
typedef struct sensors_set {
	char *name;
	sensors_expr *value;
	sensors_config_line line;
} sensors_set;

/* Config file compute declaration: a feature name, combined with two
   expressions */
typedef struct sensors_compute {
	char *name;
	sensors_expr *from_proc;
	sensors_expr *to_proc;
	sensors_config_line line;
} sensors_compute;

/* Config file ignore declaration: a feature name */
typedef struct sensors_ignore {
	char *name;
	sensors_config_line line;
} sensors_ignore;

/* A list of chip names, used to represent a config file chips declaration */
typedef struct sensors_chip_name_list {
	sensors_chip_name *fits;
	int fits_count;
	int fits_max;
} sensors_chip_name_list;

/* A config file chip block */
typedef struct sensors_chip {
	sensors_chip_name_list chips;
	sensors_label *labels;
	int labels_count;
	int labels_max;
	sensors_set *sets;
	int sets_count;
	int sets_max;
	sensors_compute *computes;
	int computes_count;
	int computes_max;
	sensors_ignore *ignores;
	int ignores_count;
	int ignores_max;
	sensors_config_line line;
} sensors_chip;

/* Config file bus declaration: the bus type and number, combined with adapter
   name */
typedef struct sensors_bus {
	char *adapter;
	sensors_bus_id bus;
	sensors_config_line line;
} sensors_bus;

/* Internal data about all features and subfeatures of a chip */
typedef struct sensors_chip_features {
	struct sensors_chip_name chip;
	struct sensors_feature *feature;
	struct sensors_subfeature *subfeature;
	int feature_count;
	int subfeature_count;
} sensors_chip_features;

#define sensors_add_config_files(el) sensors_add_array_el( \
	(el), &config->files, &config->files_count, \
	&config->files_max, sizeof(char *))

#define sensors_add_proc_chips(el) sensors_add_array_el( \
	(el), &config->features, &config->features_count,\
	&config->features_max, sizeof(struct sensors_chip_features))

#define sensors_add_proc_bus(el) sensors_add_array_el( \
	(el), &config->bus, &config->bus_count,\
	&config->bus_max, sizeof(struct sensors_bus))

typedef struct sensors_config {
const char *sysfs_mount;

char **files;
int files_count;
int files_max;

sensors_chip *chips;
int chips_count;
int chips_subst;
int chips_max;

sensors_bus *busses;
int busses_count;
int busses_max;

sensors_chip_features *features;
int features_count;
int features_max;

sensors_bus *bus;
int bus_count;
int bus_max;
} sensors_config;

extern sensors_config global_sensors_config;

/* Substitute configuration bus numbers with real-world bus numbers
   in the chips lists */
int sensors_substitute_busses(sensors_config *config);

/* Parse a bus id into its components. Returns 0 on success, a value from
   error.h on failure. */
int sensors_parse_bus_id(const char *name, sensors_bus_id *bus);

#endif /* def LIB_SENSORS_DATA_H */
