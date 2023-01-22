/*PGR-GNU*****************************************************************
 *
 F ile: trsp.c                 *                  *

 Generated with Template by:
 Copyright (c) 2013 pgRouting developers
 Mail: project@pgrouting.org

 ------

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

 ********************************************************************PGR-GNU*/

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "c_common/debug_macro.h"

#include "c_types/trsp/trsp.h"
#include "c_types/edge_t.h"

#include "c_common/edges_input.h"

#include <sqlite3.h>

#include "trsp/trsp_core.h"

typedef struct restrict_t restrict_t;
typedef struct Edge_t Edge_t;
typedef struct path_element_tt path_element_tt;

static Edge_t *edges = NULL;
static restrict_t *restricts = NULL;
static size_t total_tuples = 0;
static size_t restrict_rows = 0;

void reset_data() {
    fprintf(stderr, "reset_data\n");
    edges = NULL;
    restricts = NULL;
}

static int load_data(char* db_file, char* sql, char* restrict_sql) {
    sqlite3 *edgedb;

    int rc = sqlite3_open(db_file, &edgedb);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(edgedb));
        return -1;
    }

    if (sql == NULL) {
        fprintf(stderr, "Sql for edges is null\n");
        return -1;
    } else {
        sqlite3_stmt *stmt;
        const char* count_sql = "SELECT COUNT(*) FROM edgeTable";

        if (sqlite3_prepare(edgedb,count_sql,strlen(count_sql),&stmt,0)!=SQLITE_OK){
            fprintf(stderr, "SQL-Fehler:%s\n", sqlite3_errmsg(edgedb));
            return -1;
        }

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            total_tuples = (size_t) sqlite3_column_int(stmt,0);
        }
        fprintf(stderr, "Total %ld edge tuples\n", total_tuples);

        edges = calloc((size_t)total_tuples, sizeof(Edge_t));

        if (edges == NULL) {
            sqlite3_close(edgedb);
            fprintf(stderr, "Out of memory\n");
            return -1;
        }

        sqlite3_finalize(stmt);

        if (sqlite3_prepare(edgedb,sql,strlen(sql),&stmt,0)!=SQLITE_OK){
            fprintf(stderr, "SQL-Fehler:%s\n", sqlite3_errmsg(edgedb));
            return -1;
        }

        int num = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            Edge_t* edge = &edges[num];

            edge->id = sqlite3_column_int(stmt,0);
            edge->source = sqlite3_column_int(stmt,1);
            edge->target = sqlite3_column_int(stmt,2);
            edge->cost = sqlite3_column_double(stmt,3);
            edge->reverse_cost = sqlite3_column_double(stmt,4);

            num++;
        }
        sqlite3_finalize(stmt);
    }

    fprintf(stderr, "Fetching restriction tuples\n");

    if (restrict_sql == NULL) {
        fprintf(stderr, "Sql for restrictions is null.\n");
        return -1;
    } else {
        sqlite3_stmt *stmt;
        const char* count_sql = "SELECT COUNT(*) FROM restrictionTable";

        if (sqlite3_prepare(edgedb,count_sql,strlen(count_sql),&stmt,0)!=SQLITE_OK){
            fprintf(stderr, "SQL-Fehler:%s\n", sqlite3_errmsg(edgedb));
            return -1;
        }

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            restrict_rows = (size_t) sqlite3_column_int(stmt,0);
        }
        fprintf(stderr, "Total %ld restriction tuples\n", restrict_rows);

        restricts = calloc((size_t)restrict_rows, sizeof(restrict_t));

        if (restricts == NULL) {
            sqlite3_close(edgedb);
            fprintf(stderr, "Out of memory\n");
            return -1;
        }

        sqlite3_finalize(stmt);

        if (sqlite3_prepare(edgedb,restrict_sql,strlen(restrict_sql),&stmt,0)!=SQLITE_OK){
            fprintf(stderr, "SQL-Fehler:%s\n", sqlite3_errmsg(edgedb));
            return -1;
        }

        int num = 0;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            restrict_t* rest = &restricts[num];
            rest->target_id = sqlite3_column_int(stmt,0);
            rest->to_cost = sqlite3_column_double(stmt,1);
            const char* viaPath = sqlite3_column_text(stmt,2);

            int ci = 0;
            char* pch = (char *)strtok((char *)viaPath, " ,");

            while (pch != NULL && ci < MAX_RULE_LENGTH) {
                rest->via[ci] = atoi(pch);
                ci++;
                pch = (char *)strtok(NULL, " ,");
            }
            num++;
        }
        sqlite3_finalize(stmt);
    }
    sqlite3_close(edgedb);

    return 0;
}

path_element_tt* compute_trsp(
    char* db_file,
    char* sql,
    char* restrict_sql,
    int dovertex,
    int64_t start_id,
    double start_pos,
    int64_t end_id,
    double end_pos,
    path_element_tt **path,
    size_t *path_count) {

    bool directed = true;
    bool has_reverse_cost = true;
    char *err_msg;
    int ret = -1;

    if (restricts == NULL && edges == NULL) {
        ret = load_data(db_file, sql, restrict_sql);
        if (ret) {
            fprintf(stderr, "Failed to load data\n");
            return NULL;
        }
    }
    // defining min and max vertex id
    int64_t v_max_id = 0;
    int64_t v_min_id = INT_MAX;

    size_t z;
    for (z = 0; z < total_tuples; z++) {
        //fprintf(stderr, "id %ld source %ld target %ld cost %f rev %f",
        //        edges[z].id, edges[z].source, edges[z].target, edges[z].cost, edges[z].reverse_cost);
        if (edges[z].source < v_min_id)
            v_min_id = edges[z].source;

        if (edges[z].source > v_max_id)
            v_max_id = edges[z].source;

        if (edges[z].target < v_min_id)
            v_min_id = edges[z].target;

        if (edges[z].target > v_max_id)
            v_max_id = edges[z].target;
    }

    // ::::::::::::::::::::::::::::::::::::
    // :: reducing vertex id (renumbering)
    // ::::::::::::::::::::::::::::::::::::
    /* track if start and end are both in edge tuples */
    int s_count = 0;
    int t_count = 0;
    for (z = 0; z < total_tuples; z++) {
        // check if edges[] contains source and target
        if (dovertex) {
            if (edges[z].source == start_id || edges[z].target == start_id)
                ++s_count;
            if (edges[z].source == end_id || edges[z].target == end_id)
                ++t_count;
        } else {
            if (edges[z].id == start_id)
                ++s_count;
            if (edges[z].id == end_id)
                ++t_count;
        }

        edges[z].source -= v_min_id;
        edges[z].target -= v_min_id;
    }

    fprintf(stderr, "Min vertex id: %ld , Max vid: %ld\n", v_min_id, v_max_id);

    if (s_count == 0) {
        fprintf(stderr, "Start id was not found.\n");
        return NULL;
    }

    if (t_count == 0) {
        fprintf(stderr, "Target id was not found.\n");
        return NULL;
    }

    if (dovertex) {
        start_id -= v_min_id;
        end_id   -= v_min_id;
    }

    fprintf(stderr, "Calling trsp_edge_wrapper\n");
    ret = trsp_edge_wrapper(edges, total_tuples,
                            restricts, restrict_rows,
                            start_id, start_pos, end_id, end_pos,
                            directed, has_reverse_cost,
                            path, path_count, &err_msg);


    if (ret < 0) {
        fprintf(stderr, "Error computing path ret = %d\n", ret);
    } else {
        // ::::::::::::::::::::::::::::::::
        // :: restoring original vertex id
        // ::::::::::::::::::::::::::::::::
        for (z = 0; z < *path_count; z++) {
            if (z || (*path)[z].vertex_id != -1)
                (*path)[z].vertex_id += v_min_id;
        }
        /*fprintf(stderr, "path_count = %ld\n", *path_count);
        fprintf(stderr, "(");
        for (z = 0; z < *path_count; z++) {
            fprintf(stderr, "%d,", (*path)[z].edge_id);
        }
        fprintf(stderr, ")\n");*/
    }

    return *path;
}
