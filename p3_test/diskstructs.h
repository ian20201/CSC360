// Super block
struct __attribute__((__packed__)) superblock_t {
  uint8_t fs_id [8];
  uint16_t block_size;
  uint32_t file_system_block_count;
  uint32_t fat_start_block;
  uint32_t fat_block_count;
  uint32_t root_dir_start_block;
  uint32_t root_dir_block_count;
};

// Time and date entry
struct __attribute__((__packed__)) dir_entry_timedate_t {
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
};

// Directory entry
struct __attribute__((__packed__)) dir_entry_t {
  uint8_t status;
  uint32_t starting_block;
  uint32_t block_count;
  uint32_t size;
  struct dir_entry_timedate_t create_time;
  struct dir_entry_timedate_t modify_time;
  uint8_t filename[31];
  uint8_t unused[6];
};
