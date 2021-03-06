/* 
   DataFlash logging - file oriented variant

   This uses posix file IO to create log files called logNN.dat in the
   given directory
 */
#ifndef __DataFlash_DataFlash_File_h__
#define __DataFlash_DataFlash_File_h__


#if HAL_OS_POSIX_IO

#include <AP_HAL/utility/RingBuffer.h>
#include "DataFlash_Backend.h"

#if CONFIG_HAL_BOARD == HAL_BOARD_QURT
/*
  the QURT port has a limited range of system calls available. It
  cannot provide all the facilities that DataFlash_File wants. It can
  provide enough to be useful though, which is what
  DATAFLASH_FILE_MINIMAL is for
 */
#define DATAFLASH_FILE_MINIMAL 1
#else
#define DATAFLASH_FILE_MINIMAL 0
#endif

class DataFlash_File : public DataFlash_Backend
{
public:
    // constructor
    DataFlash_File(DataFlash_Class &front,
                   DFMessageWriter_DFLogStart *,
                   const char *log_directory);

    // initialisation
    void Init() override;
    bool CardInserted(void) override;

    // erase handling
    void EraseAll() override;

    // possibly time-consuming preparation handling:
    bool NeedPrep() override;
    void Prep() override;

    /* Write a block of data at current offset */
    bool WritePrioritisedBlock(const void *pBuffer, uint16_t size, bool is_critical) override;
    uint32_t bufferspace_available() override;

    // high level interface
    uint16_t find_last_log() override;
    void get_log_boundaries(uint16_t log_num, uint16_t & start_page, uint16_t & end_page) override;
    void get_log_info(uint16_t log_num, uint32_t &size, uint32_t &time_utc) override;
    int16_t get_log_data(uint16_t log_num, uint16_t page, uint32_t offset, uint16_t len, uint8_t *data) override;
    uint16_t get_num_logs() override;
    uint16_t start_new_log(void) override;
    void LogReadProcess(const uint16_t log_num,
                        uint16_t start_page, uint16_t end_page, 
                        print_mode_fn print_mode,
                        AP_HAL::BetterStream *port) override;
    void DumpPageInfo(AP_HAL::BetterStream *port) override;
    void ShowDeviceInfo(AP_HAL::BetterStream *port) override;
    void ListAvailableLogs(AP_HAL::BetterStream *port) override;

#if CONFIG_HAL_BOARD == HAL_BOARD_SITL || CONFIG_HAL_BOARD == HAL_BOARD_LINUX
    void flush(void) override;
#endif
    void periodic_1Hz(const uint32_t now) override;
    void periodic_fullrate(const uint32_t now) override;

    // this method is used when reporting system status over mavlink
    bool logging_enabled() const override;
    bool logging_failed() const override;

    void vehicle_was_disarmed() override;

protected:

    bool WritesOK() const override;

private:
    int _write_fd;
    int _read_fd;
    uint16_t _read_fd_log_num;
    uint32_t _read_offset;
    uint32_t _write_offset;
    volatile bool _initialised;
    volatile bool _open_error;
    const char *_log_directory;

    uint32_t _io_timer_heartbeat;
    bool io_thread_alive() const;
    uint8_t io_thread_warning_decimation_counter;

    uint16_t _cached_oldest_log;

    /*
      read a block
    */
    bool ReadBlock(void *pkt, uint16_t size) override;

    uint16_t _log_num_from_list_entry(const uint16_t list_entry);

    // possibly time-consuming preparations handling
    void Prep_MinSpace();
    uint16_t find_oldest_log();
    int64_t disk_space_avail();
    int64_t disk_space();
    float avail_space_percent();

    bool file_exists(const char *filename) const;
    bool log_exists(const uint16_t lognum) const;

#if CONFIG_HAL_BOARD == HAL_BOARD_SITL || CONFIG_HAL_BOARD == HAL_BOARD_LINUX
    // I always seem to have less than 10% free space on my laptop:
    const float min_avail_space_percent = 0.1f;
#else
    const float min_avail_space_percent = 10.0f;
#endif
    // write buffer
    ByteBuffer _writebuf;
    const uint16_t _writebuf_chunk;
    uint32_t _last_write_time;

    /* construct a file name given a log number. Caller must free. */
    char *_log_file_name(const uint16_t log_num) const;
    char *_log_file_name_long(const uint16_t log_num) const;
    char *_log_file_name_short(const uint16_t log_num) const;
    char *_lastlog_file_name() const;
    uint32_t _get_log_size(const uint16_t log_num) const;
    uint32_t _get_log_time(const uint16_t log_num) const;

    void stop_logging(void) override;

    void _io_timer(void);

    uint32_t critical_message_reserved_space() const {
        // possibly make this a proportional to buffer size?
        uint32_t ret = 1024;
        if (ret > _writebuf.get_size()) {
            // in this case you will only get critical messages
            ret = _writebuf.get_size();
        }
        return ret;
    };
    uint32_t non_messagewriter_message_reserved_space() const {
        // possibly make this a proportional to buffer size?
        uint32_t ret = 1024;
        if (ret >= _writebuf.get_size()) {
            // need to allow messages out from the messagewriters.  In
            // this case while you have a messagewriter you won't get
            // any other messages.  This should be a corner case!
            ret = 0;
        }
        return ret;
    };

    // free-space checks; filling up SD cards under NuttX leads to
    // corrupt filesystems which cause loss of data, failure to gather
    // data and failures-to-boot.
    uint32_t _free_space_last_check_time; // milliseconds
    const uint32_t _free_space_check_interval = 1000UL; // milliseconds
    const uint32_t _free_space_min_avail = 8388608; // bytes

    // semaphore mediates access to the ringbuffer
    AP_HAL::Semaphore *semaphore;
    // write_fd_semaphore mediates access to write_fd so the frontend
    // can open/close files without causing the backend to write to a
    // bad fd
    AP_HAL::Semaphore *write_fd_semaphore;
    
    // performance counters
    AP_HAL::Util::perf_counter_t  _perf_write;
    AP_HAL::Util::perf_counter_t  _perf_fsync;
    AP_HAL::Util::perf_counter_t  _perf_errors;
    AP_HAL::Util::perf_counter_t  _perf_overruns;
};

#endif // HAL_OS_POSIX_IO

#endif /* __DataFlash_DataFlash_File_h__ */