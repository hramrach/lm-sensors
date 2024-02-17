/*
    conf.h - Part of libsensors, a Linux library for reading sensor data.
    Copyright (c) 1998, 1999  Frodo Looijaard <frodol@dds.nl>

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

#ifndef LIB_SENSORS_CONF_H
#define LIB_SENSORS_CONF_H

typedef void *yyscan_t;
typedef struct yy_buffer_state *YY_BUFFER_STATE;

typedef struct extra {
int buffer_count;
int buffer_max;
char *buffer;
const char *lex_error;
const char *filename;
int lineno;
YY_BUFFER_STATE scan_buf;
} extra;

typedef union YYSTYPE YYSTYPE;

/* This is defined in conf-lex.l */
int sensors_yylex(YYSTYPE *yylval_param, yyscan_t yyscanner);

/* This is defined in conf-parse.y */
int sensors_yyparse(sensors_config *config, sensors_chip *current_chip, yyscan_t yyscanner);

/* This is defined in strtod/strtod.c */
double fmtstrtod(const char *as, const char **aas);

#endif /* def LIB_SENSORS_CONF_H */
