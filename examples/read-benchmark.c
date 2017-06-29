#include <arpa/inet.h>
#include <dirent.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <tg-lib/tracer.h>
#include <tg-lib/try.h>
#include <tg-lib/util.h>
#include <unistd.h>

#ifdef ENABLE_MILLDB
#include "filter/ipmilldb.h"
#else
#include "database/database_read.h"
#endif


#ifndef ENABLE_MILLDB
// Copied from tg-tool_var.h
uint64_t
get_file_size(char *name)
{
  struct stat st;
  TRY(stat(name, &st), != 0, ABORT, "Cannot stat database file: %m");
  return st.st_size;
}

//Database size is stored in database.size file.
//If that file is missing, just read the file size
uint64_t
get_db_size(const char *dirname)
{
  char *fs_name;
  uint64_t db_size;
  FILE *fp_size;

  db_size = 0;
  TRY(asprintf(&fs_name, "%s/database.size", dirname), == -1, ABORT, "%m");
  fp_size = fopen(fs_name, "r");
  free(fs_name);
  if (fp_size != NULL)
    {
      TRY(fscanf(fp_size, "%"SCNu64, &db_size), != 1,
          ABORT, "Wrong database.size file");
      fclose(fp_size);
    }
  else
    {
      TRY(asprintf(&fs_name, "%s/database", dirname), == -1, ABORT, "%m");
      db_size = get_file_size(fs_name);
      free(fs_name);
    }
  return db_size;
}
#endif  /* ifndef ENABLE_MILLDB */


#define IP_MASK "%d.%d.%d.%d"
#define IP(val) \
  ((val) >> 24) & 0xFF, ((val) >> 16) & 0xFF, ((val) >> 8) & 0xFF, (val) & 0xFF


#ifndef ENABLE_MILLDB
static
bool
d_records_func(void *arg, const struct progress *progress, struct d_record *d)
{
  DONT_WARN_ABOUT(arg);
  DONT_WARN_ABOUT(progress);

  if (! d)
    return false;

  if (d->version != T_V1_TCP && d->version != T_V1_UDP)
    {
      printf("unexpected record %d\n", (int) d->version);
      return false;
    }

  printf("src="IP_MASK":%"PRIu16", dst="IP_MASK":%"PRIu16", "
	 "ts=%"PRIu32", proto=%d, send=%"PRIu32", rec=%"PRIu32"\n",
	 IP(d->s), ntohs(d->sp), IP(d->d), ntohs(d->dp),
	 d->ts, (int) d->other_tcp_udp.protocol,
	 (uint32_t) d->other_tcp_udp.tx,
	 (uint32_t) d->other_tcp_udp.rx);

  return false;
}
#endif  /* ifndef ENABLE_MILLDB */


int
main(int argc, char *argv[])
{
  if (argc != 4)
    ABORT("command line:\nread_benchmark_... ip_file.bin tdb_dir milldb_file");

  const char *ip_file_name = argv[1];
  const char *tdb_dir_name = argv[2];
  const char *milldb_file = argv[3];

  FILE *ips = TRY(fopen(ip_file_name, "rb"), == NULL, ABORT, "%m");

#ifdef ENABLE_MILLDB
  DONT_WARN_ABOUT(tdb_dir_name);
  struct ipmilldb_handle *handle =
    TRY(ipmilldb_open_read(milldb_file), == NULL, ABORT, "%m");
#else
  DIR *d = TRY(opendir(tdb_dir_name), == NULL, ABORT, "%m");
  int root_fd = SYS(dirfd(d));
  struct database_handle *handle =
    database_open_read(root_fd, "database", get_db_size(tdb_dir_name));
  DONT_WARN_ABOUT(milldb_file);
#endif


  uint32_t ip;
  while (fread(&ip, sizeof(ip), 1, ips) == 1)
    {
      printf("IP: "IP_MASK"\n", IP(ip));
#ifdef ENABLE_MILLDB
      struct get_by_src_ip_out src_iter;
      get_by_src_ip_init(&src_iter, handle, ip);
      while (get_by_src_ip_next(&src_iter))
	{
	  printf("src="IP_MASK":%"PRIu16", dst="IP_MASK":%"PRIu16", "
		 "ts=%"PRIu32", proto=%d, send=%"PRIu32", rec=%"PRIu32"\n",
		 IP(ip), ntohs(src_iter.data.src_port),
		 IP(src_iter.data.dst_ip), ntohs(src_iter.data.dst_port),
		 (uint32_t) src_iter.data.timestamp,
		 (int) src_iter.data.protocol,
		 (uint32_t) src_iter.data.send_bytes,
		 (uint32_t) src_iter.data.receive_bytes);

	}

      struct get_reverse_ip_out rev_iter;
      get_reverse_ip_init(&rev_iter, handle, ip);
      while (get_reverse_ip_next(&rev_iter))
	{
	  struct get_by_src_ip_out src_iter;
	  get_by_src_ip_init(&src_iter, handle, rev_iter.data.src_ip);
	  while (get_by_src_ip_next(&src_iter))
	    {
	      printf("src="IP_MASK":%"PRIu16", dst="IP_MASK":%"PRIu16", "
		     "ts=%"PRIu32", proto=%d, send=%"PRIu32", rec=%"PRIu32"\n",
		     IP(rev_iter.data.src_ip), ntohs(src_iter.data.src_port),
		     IP(src_iter.data.dst_ip), ntohs(src_iter.data.dst_port),
		     (uint32_t) src_iter.data.timestamp,
		     (int) src_iter.data.protocol,
		     (uint32_t) src_iter.data.send_bytes,
		     (uint32_t) src_iter.data.receive_bytes);
	    }
	}
#else
      database_process_d_records(handle, 0, 0, (uint32_t) -1, ip, OOB_LAG, true,
				 NULL, d_records_func);
#endif
    }


#ifdef ENABLE_MILLDB
  ipmilldb_close_read(handle);
#else
  database_close_read(handle);
  closedir(d);
#endif

  FSYS(fclose(ips));

  return 0;
}
