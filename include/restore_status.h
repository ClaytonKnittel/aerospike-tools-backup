/*
 * Aerospike Restore Status
 *
 * Copyright (c) 2022 Aerospike, Inc. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do
 * so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

//==========================================================
// Includes.
//

#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wsign-conversion"

#include <aerospike/as_node.h>

#pragma GCC diagnostic warning "-Wconversion"
#pragma GCC diagnostic warning "-Wsign-conversion"

#include <restore_config.h>
#include <utils.h>


//==========================================================
// Typedefs & constants.
//

/*
 * The restore_status_t struct is used to manage the status of a full restore job.
 */
typedef struct restore_status {
	// The Aerospike client.
	aerospike* as;

	// The file format decoder to be used for reading data from a backup file.
	backup_decoder_t decoder;

	// The list of backup files to restore.
	as_vector file_vec;
	// The (optional) source and (also optional) target namespace to be
	// restored, as a vector of strings.
	as_vector ns_vec;
	// The bins to be restored, as a vector of bin name strings.
	as_vector bin_vec;
	// The sets to be restored, as a vector of set name strings.
	as_vector set_vec;
	// The indexes to be inserted, as a vector of index_param's
	as_vector index_vec;
	// The udfs to be inserted, as a vector of udf_param's
	as_vector udf_vec;

	// Mutex for exclusive access to index_vec/udf_vec
	pthread_mutex_t idx_udf_lock;

	// The total size of all backup files to be restored.
	off_t estimated_bytes;
	// The total number of bytes read from the backup file(s) so far.
	uint64_t total_bytes;
	// The total number of records read from the backup file(s) so far.
	uint64_t total_records;
	// The number of records dropped because they were expired.
	uint64_t expired_records;
	// The number of records dropped because they didn't contain any of the
	// selected bins or didn't belong to any of the the selected sets.
	uint64_t skipped_records;
	// The number of records ignored because of record level permanent error while
	// restoring. e.g RECORD_TOO_BIG Enabled or disabled using
	// --ignore-record-error flag.
	uint64_t ignored_records;
	// The number of successfully restored records.
	uint64_t inserted_records;
	// The number of records dropped because they already existed in the
	// database.
	uint64_t existed_records;
	// The number of records dropped because the database already contained the
	// records with a higher generation count.
	uint64_t fresher_records;
	// How often we backed off due to server overload.
	uint64_t backoff_count;
	// The current limit for total_bytes for throttling. This is periodically
	// increased by the counter thread to raise the limit according to the
	// bandwidth limit.
	volatile uint64_t bytes_limit;
	// The current limit for total_records for throttling.
	// This is periodically increased by the counter thread to raise the limit
	// according to the TPS limit.
	volatile uint64_t records_limit;
	// The number of successfully created secondary indexes.
	uint32_t index_count;
	// counts of the number of inserted/skipped/matched/mismatched secondary indexes
	uint32_t skipped_indexes;
	uint32_t matched_indexes;
	uint32_t mismatched_indexes;
	// The number of successfully stored UDF files.
	uint32_t udf_count;
} restore_status_t;


//==========================================================
// Public API.
//

bool restore_status_init(restore_status_t*, const restore_config_t*);

void restore_status_destroy(restore_status_t*);

