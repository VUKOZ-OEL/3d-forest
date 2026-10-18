/* Run from a disposable build-directory copy of the model. */
#include "runilandmodel.h"
#include "sqlite3.h"
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void fail(const char *message) {
    fprintf(stderr, "ERROR: %s\n", message);
    exit(2);
}

static sqlite3 *open_db(const char *path) {
    sqlite3 *db = NULL;
    if (sqlite3_open_v2(path, &db, SQLITE_OPEN_READONLY, NULL) != SQLITE_OK)
        fail(db ? sqlite3_errmsg(db) : "Cannot open database");
    return db;
}

static sqlite3_stmt *prepare(sqlite3 *db, const char *sql) {
    sqlite3_stmt *s = NULL;
    if (sqlite3_prepare_v2(db, sql, -1, &s, NULL) != SQLITE_OK)
        fail(sqlite3_errmsg(db));
    return s;
}

static void execute(sqlite3 *db, const char *sql) {
    if (sqlite3_exec(db, sql, NULL, NULL, NULL) != SQLITE_OK)
        fail(sqlite3_errmsg(db));
}

/* The supplied CSV is unquoted, semicolon-separated numeric data. */
static const char *csv_column(const char *name) {
    /* Legacy and current names for the same quantities in Permafrost::debugData. */
    static const char *aliases[][2] = {
        {"pftop", "pf_top"}, {"pfbottom", "pf_bottom"},
        {"pffreezeback", "pf_freezeback"}, {"delta_mm", "pf_delta_water_mm"},
        {"delta_soil", "pf_delta_soil_m"}, {"thermalConductivity", "pf_k_unfrozen"},
        {"soilfrozen", "pf_soil_frozen_m"}, {"waterfrozen", "pf_water_frozen_mm"},
        {"current_capacity", "pf_unfrozen_fc_mm"}, {"moss_fLight", "moss_f_light"},
        {"moss_fDecid", "moss_f_deciduous"}
    };
    for (size_t i = 0; i < sizeof(aliases) / sizeof(aliases[0]); ++i)
        if (strcmp(name, aliases[i][0]) == 0) return aliases[i][1];
    return name;
}

static sqlite3 *read_csv(const char *path) {
    FILE *f = fopen(path, "r");
    char line[16384], *token, *query;
    int columns = 0;
    sqlite3 *db = NULL;
    sqlite3_str *sql;
    sqlite3_stmt *insert;
    if (!f) fail(path);
    if (sqlite3_open(":memory:", &db) != SQLITE_OK) fail("CSV database allocation");
    if (!fgets(line, sizeof(line), f)) fail("Empty CSV");
    sql = sqlite3_str_new(db);
    sqlite3_str_appendall(sql, "CREATE TABLE daily(");
    for (token = strtok(line, ";\r\n"); token; token = strtok(NULL, ";\r\n"))
        sqlite3_str_appendf(sql, "%s\"%w\" REAL", columns++ ? "," : "", csv_column(token));
    sqlite3_str_appendall(sql, ")");
    query = sqlite3_str_finish(sql);
    execute(db, query);
    sqlite3_free(query);
    sql = sqlite3_str_new(db);
    sqlite3_str_appendall(sql, "INSERT INTO daily VALUES(");
    for (int i = 0; i < columns; ++i) sqlite3_str_appendall(sql, i ? ",?" : "?");
    sqlite3_str_appendall(sql, ")");
    query = sqlite3_str_finish(sql);
    insert = prepare(db, query);
    sqlite3_free(query);
    execute(db, "BEGIN");
    while (fgets(line, sizeof(line), f)) {
        int n = 0;
        if (!strchr(line, '\n') && !feof(f)) fail("CSV line too long");
        for (token = strtok(line, ";\r\n"); token; token = strtok(NULL, ";\r\n")) {
            char *end;
            double value = strtod(token, &end);
            if (*end || end == token || !isfinite(value) || n >= columns)
                fail("Invalid numeric CSV field");
            sqlite3_bind_double(insert, ++n, value);
        }
        if (n != columns) fail("Missing CSV fields");
        if (sqlite3_step(insert) != SQLITE_DONE) fail(sqlite3_errmsg(db));
        sqlite3_reset(insert);
    }
    if (ferror(f)) fail("CSV read error");
    fclose(f);
    sqlite3_finalize(insert);
    execute(db, "COMMIT");
    return db;
}

static int equal_value(sqlite3_stmt *a, sqlite3_stmt *b, int col, int exact) {
    int ta = sqlite3_column_type(a, col), tb = sqlite3_column_type(b, col);
    if ((ta == SQLITE_INTEGER || ta == SQLITE_FLOAT) &&
        (tb == SQLITE_INTEGER || tb == SQLITE_FLOAT)) {
        double x = sqlite3_column_double(a, col), y = sqlite3_column_double(b, col);
        return isfinite(x) && isfinite(y) &&
            (exact ? x == y : fabs(x-y) <= 1e-6 + 1e-6 * fmax(fabs(x), fabs(y)));
    }
    if (ta != tb) return 0;
    if (ta == SQLITE_NULL) return 1;
    return strcmp((const char *)sqlite3_column_text(a, col),
                  (const char *)sqlite3_column_text(b, col)) == 0;
}

static int compare(sqlite3 *expected, sqlite3 *actual, const char *table,
                   const char *keys, int key_columns) {
    char *query = sqlite3_mprintf("SELECT * FROM \"%w\" LIMIT 0", table);
    sqlite3_stmt *schema = prepare(expected, query), *a, *b;
    sqlite3_str *sql = sqlite3_str_new(expected);
    int columns = sqlite3_column_count(schema), ra, rb;
    long rows = 0, mismatches = 0;
    sqlite3_free(query);
    sqlite3_str_appendall(sql, "SELECT ");
    /* Missing reference columns fail. New columns have no reference values. */
    for (int i = 0; i < columns; ++i)
        sqlite3_str_appendf(sql, "%s[%s]", i ? "," : "", sqlite3_column_name(schema, i));
    sqlite3_str_appendf(sql, " FROM \"%w\" ORDER BY %s", table, keys);
    query = sqlite3_str_finish(sql);
    a = prepare(expected, query);
    b = prepare(actual, query);
    sqlite3_free(query);
    sqlite3_finalize(schema);
    for (;;) {
        ra = sqlite3_step(a);
        rb = sqlite3_step(b);
        if ((ra != SQLITE_ROW && ra != SQLITE_DONE) || (rb != SQLITE_ROW && rb != SQLITE_DONE))
            fail("Error reading comparison rows");
        if (ra == SQLITE_DONE && rb == SQLITE_DONE) break;
        ++rows;
        if (ra != rb) {
            sqlite3_stmt *rest = ra == SQLITE_ROW ? a : b;
            fprintf(stderr, "%s: extra/missing rows starting at %ld\n", table, rows);
            ++mismatches;
            while ((ra = sqlite3_step(rest)) == SQLITE_ROW) ++mismatches;
            if (ra != SQLITE_DONE) fail("Error reading remaining rows");
            break;
        }
        for (int i = 0; i < columns; ++i) {
            if (!equal_value(a, b, i, i < key_columns)) {
                if (mismatches < 5)
                    fprintf(stderr, "%s row %ld %s: expected %s, actual %s\n", table, rows,
                        sqlite3_column_name(a, i),
                        sqlite3_column_type(a, i) == SQLITE_NULL ? "NULL" : (const char *)sqlite3_column_text(a, i),
                        sqlite3_column_type(b, i) == SQLITE_NULL ? "NULL" : (const char *)sqlite3_column_text(b, i));
                ++mismatches;
            }
        }
    }
    if (!rows) { fprintf(stderr, "%s: empty reference table\n", table); ++mismatches; }
    printf("%s: %ld rows checked, %ld differences\n", table, rows, mismatches);
    sqlite3_finalize(a);
    sqlite3_finalize(b);
    return mismatches != 0;
}

static void remove_output(const char *path) {
    if (remove(path) && errno != ENOENT) fail("Cannot remove previous calculated output");
}

int main(int argc, char **argv) {
    char *path;
    sqlite3 *expected, *actual;
    FILE *marker;
    int failed = 0;
    if (argc != 2 && !(argc == 3 && strcmp(argv[2], "--compare-only") == 0)) {
        fprintf(stderr, "Usage: iland-permafrost-benchmark <reference-model-directory> [--compare-only]\nRun in the build-directory input copy.\n");
        return 2;
    }
    /* The original model never has this marker. */
    marker = fopen(".iland-benchmark-copy", "r");
    if (!marker) fail("Run only in the prepared build-directory copy");
    fclose(marker);
    if (argc == 2) {
        remove_output("output/output.sqlite");
        remove_output("temp/debug_water_cycle.csv");
        /* The API uses an exclusive final year: 19 runs years 1 through 18. */
        if (runilandmodel("permafrost_bore.hole_project_file.xml", 19) != 0)
            fail("runilandmodel failed; see the copied model's log.txt");
    }
    path = sqlite3_mprintf("%s/output/output.sqlite", argv[1]);
    expected = open_db(path);
    sqlite3_free(path);
    actual = open_db("output/output.sqlite");
    failed |= compare(expected, actual, "water", "year,ru,rid", 3);
    failed |= compare(expected, actual, "stand", "year,ru,rid,species", 4);
    failed |= compare(expected, actual, "sapling", "year,ru,rid,species", 4);
    failed |= compare(expected, actual, "saplingdetail", "year,ru,rid,species,position", 5);
    sqlite3_close(expected);
    sqlite3_close(actual);
    path = sqlite3_mprintf("%s/temp/debug_water_cycle.csv", argv[1]);
    expected = read_csv(path);
    sqlite3_free(path);
    actual = read_csv("temp/debug_water_cycle.csv");
    failed |= compare(expected, actual, "daily", "year,ruindex,date,id,type,rid", 6);
    sqlite3_close(expected);
    sqlite3_close(actual);
    puts(failed ? "FAIL: calculated results differ from the supplied reference." : "PASS: calculated results match the supplied reference.");
    return failed ? 1 : 0;
}
