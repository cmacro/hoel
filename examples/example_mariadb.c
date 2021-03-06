/**
 * 
 * Example program using MariaDB to execute SQL statements
 * 
 * Copyright 2016-2018 Nicolas Mora <mail@babelouest.org>
 *
 * License: MIT
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <jansson.h>
#define _HOEL_MARIADB
#include <hoel.h>

void print_result(struct _h_result result) {
  int col, row;
  char buf[64];
  int i;
  printf("rows: %u, col: %u\n", result.nb_rows, result.nb_columns);
  for (row = 0; row<result.nb_rows; row++) {
    for (col=0; col<result.nb_columns; col++) {
      switch(result.data[row][col].type) {
        case HOEL_COL_TYPE_INT:
          printf("| %d ", ((struct _h_type_int *)result.data[row][col].t_data)->value);
          break;
        case HOEL_COL_TYPE_DOUBLE:
          printf("| %f ", ((struct _h_type_double *)result.data[row][col].t_data)->value);
          break;
        case HOEL_COL_TYPE_TEXT:
          printf("| %s ", ((struct _h_type_text *)result.data[row][col].t_data)->value);
          break;
        case HOEL_COL_TYPE_BLOB:
          for (i=0; i<((struct _h_type_blob *)result.data[row][col].t_data)->length; i++) {
            printf("%c", *((char*)(((struct _h_type_blob *)result.data[row][col].t_data)->value+i)));
            if (i%80 == 0 && i>0) {
              printf("\n");
            }
          }
          break;
        case HOEL_COL_TYPE_DATE:
          strftime(buf, 64, "%Y-%m-%d %H:%M:%S", &((struct _h_type_datetime *)result.data[row][col].t_data)->value);
          printf("| %s ", buf);
        case HOEL_COL_TYPE_NULL:
          printf("| [null] ");
          break;
      }
    }
    printf("|\n");
  }
}

void unit_tests(struct _h_connection * conn) {
  json_t * j_result;
  struct _h_result result;
  struct _h_data * data;
  char * query = NULL, * sanitized = NULL, * dump, * table = "other_test";
  int last_id = -1;
  
  // Execute a sql query and retrieve results in a json structure
  query = msprintf("select * from %s", table);
  if (h_query_select_json(conn, query, &j_result) == H_OK) {
    dump = json_dumps(j_result, JSON_INDENT(2));
    printf("json result is\n%s\n", dump);
    json_decref(j_result);
    free(dump);
  } else {
    printf("Error executing query\n");
  }
  free(query);
  
  // insert escaped string
  sanitized = h_escape_string(conn, "Hodor son of H'rtp'ss");
  query = msprintf("insert into %s (name, age, temperature, birthdate) values ('%s', %d, %f, '%s')", table, sanitized, 33, 37.2, "1412-03-08 12:00:22");
  printf("insert result: %d\n", h_query_insert(conn, query));
  free(sanitized);
  free(query);
  
  query = msprintf("select * from %s", table);
  if (h_query_select(conn, query, &result) == H_OK) {
    print_result(result);
    h_clean_result(&result);
  } else {
    printf("Error executing query\n");
  }
  free(query);
  
  sanitized = h_escape_string(conn, "Ygritte you know nothing");
  query = msprintf("insert into %s (name, age, temperature, birthdate) values ('%s', %d, %f, '%s')", table, sanitized, 25, 30.1, "1424-06-01 03:05:11");
  printf("insert result: %d\n", h_query_insert(conn, query));
  free(sanitized);
  free(query);
  
  query = msprintf("select * from %s", table);
  if (h_query_select(conn, query, &result) == H_OK) {
    print_result(result);
    h_clean_result(&result);
  } else {
    printf("Error executing query\n");
  }
  free(query);
  
  sanitized = h_escape_string(conn, "Littlefinger I will betray you");
  query = msprintf("insert into %s (name, age, temperature, birthdate) values ('%s', %d, %f, '%s')", table, sanitized, 44, 40.5, "1410-10-25 14:30:00");
  printf("insert result: %d\n", h_query_insert(conn, query));
  free(sanitized);
  free(query);
  data = h_query_last_insert_id(conn);
  if (data->type == HOEL_COL_TYPE_INT) {
    last_id = ((struct _h_type_int *)data->t_data)->value;
  }
  h_clean_data_full(data);
  printf("last id is %d\n", last_id);
  
  query = msprintf("select * from %s", table);
  if (h_query_select(conn, query, &result) == H_OK) {
    print_result(result);
    h_clean_result(&result);
  } else {
    printf("Error executing query\n");
  }
  free(query);
  
  sanitized = h_escape_string(conn, "Littlefinger I am nothing");
  query = msprintf("update %s set name='%s' where id=%d", table, sanitized, last_id);
  printf("update result: %d\n", h_query_update(conn, query));
  free(sanitized);
  free(query);

  query = msprintf("select * from %s", table);
  if (h_query_select(conn, query, &result) == H_OK) {
    print_result(result);
    h_clean_result(&result);
  } else {
    printf("Error executing query\n");
  }
  free(query);
  
  query = msprintf("delete from %s where id=%d", table, last_id);
  printf("delete result: %d\n", h_query_delete(conn, query));
  free(query);
  
  query = msprintf("select * from %s", table);
  if (h_query_select_json(conn, query, &j_result) == H_OK) {
    dump = json_dumps(j_result, JSON_INDENT(2));
    printf("json result is\n%s\n", dump);
    json_decref(j_result);
    free(dump);
  } else {
    printf("Error executing query\n");
  }
  free(query);
}

int main(int argc, char ** argv) {
  struct _h_connection * conn;
  
  conn = h_connect_mariadb("localhost", "test_hoel", "test_hoel", "test_hoel", 0, NULL);
  
  if (conn != NULL) {
    unit_tests(conn);
  } else {
    printf("Error connecting to database\n");
  }
  h_close_db(conn);
  return h_clean_connection(conn);
}
