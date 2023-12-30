#ifndef TAGFI_FIOPS_H
#define TAGFI_FIOPS_H


#define ANCHORNMLEN 65

// Config byte markers:
#define LEAD_BMARK 0x1f10081f
#define LEN_HEAD_BMARK 0x7f40207f
#define LEN_TAIL_BMARK 0x1ff601ff
#define ITEM_VALS_BMARK 0xffef0b03
#define ITEM_VALS_SUBLIST_BMARK 0xc0600c18
#define TERMINATOR_BMARK 0x31a81431
#define DELIM_BMARK 0x00
#define CONFIG_VAL_PLACEHOLDER 0xff
#define CONFIG_BITMAP 1024
#define CONFIG_OUTPUT_BUF_SZ 2048
#define CONFIG_TMP_BUF_SZ 256
#define BMARK_CNT 7
#define BMARK_WIDTH 4

#define TYPE_CNT 8
#define VOID_SIZE 0
#define UINT_SIZE sizeof(unsigned int)
#define UCHAR_SIZE sizeof(unsigned char)
#define ULONG_SIZE sizeof(unsigned long)
#define INT_SIZE sizeof(int)
#define CHAR_SIZE sizeof(char)
#define LONG_SIZE sizeof(long)

extern const unsigned int type_size_arr[TYPE_CNT];
typedef const unsigned int (*TypeSizeArr)[TYPE_CNT];
extern TypeSizeArr typesizes;
extern const unsigned int byte_marks_source[BMARK_CNT];
typedef const unsigned int (*ByteMarkArr)[BMARK_CNT];
extern ByteMarkArr bytemarks;

extern const unsigned int bytemark_sz;
typedef const int TempValue;
extern TempValue tempVal;

/*
 * CONFLEAD = 0,
 * LENHEAD = 1,
 * LENTAIL = 2,
 * ITMVALS = 3,
 * SUBLIST = 4,
 * TERMINTR = 5,
 * DELIMNTR = 6
 */
typedef enum Bytemark_Flags{
    CONFLEAD = 0,
    LENHEAD = 1,
    LENTAIL = 2,
    ITMVALS = 3,
    SUBLIST = 4,
    TERMINTR = 5,
    DELIMNTR = 6
}Bytemark_Flags;

/*
 *  VOID_F = 0,
 *  UINT_F = 1,
 *  UCHAR_F = 2,
 *  ULONG_F = 3,
 *  INT_F = 4,
 *  CHAR_F = 5,
 *  LONG_F = 6,
 *  PTR_F = 7
 */
typedef enum TypeFlags{
    VOID_F = 0,
    UINT_F = 1,
    UCHAR_F = 2,
    ULONG_F = 3,
    INT_F = 4,
    CHAR_F = 5,
    LONG_F = 6
}TypeFlags;

typedef struct ConfigMultiTool* ConfigMT;


typedef unsigned int (*produce_bytemark)(Bytemark_Flags);
typedef unsigned int (*produce_typesize)(TypeFlags);
typedef unsigned long (*pos_advancment)(unsigned long, ConfigMT);
typedef void (*buffer_insert)(const void*, size_t, ConfigMT);
typedef void (*add_buffer_element)(ConfigMT);
typedef void (*fill_with_tempval)(ConfigMT, unsigned long);
typedef void (*add_buffer_marker)(ConfigMT, Bytemark_Flags);



typedef struct ConfigMultiTool{
    unsigned long pos;
    ByteMarkArr byteMarkArr;
    TypeSizeArr typeSizeArr;
    TempValue* placeHolder;
    Bytemark_Flags bytemarkFlag;
    Bytemark_Flags delim;
    TypeFlags typeSizeFlag;
    unsigned char* output_buf;
    unsigned int conf_bitmap;
    produce_bytemark ret_bytemark;
    produce_typesize ret_typesize;
    pos_advancment  adv_positon;
    buffer_insert into_buffer;
    fill_with_tempval fill_with_temp;
    add_buffer_element add_delim;
    add_buffer_element add_newline;
    add_buffer_element add_conf_lead;
    add_buffer_element add_len_seg;
    add_buffer_marker add_marker;

    unsigned long item_cnt;

}ConfigMultiTool;

typedef enum StreamMode{
    // "r"
    r_STRM = 1,
    // "w"
    w_STRM = 2,
    // "a"
    a_STRM = 3,
    // "r+"
    R_STRM = 4,
    // "w+"
    W_STRM = 5,
    // "a+"
    A_STRM = 6
}StreamMode;

typedef struct PathParts {
    unsigned int namelen;
    unsigned int pathlen;
    unsigned int resdirname_len;
    unsigned char* name;
    const char* parentpath;
    char* resdir_name;
}PathParts;

typedef struct Lattice_FD{
    int prime_fd;
    int duped_fd;
    int o_flgs;
    int modes;
    int dir_fd;
    int tag; // 0 = none, 1 = reg, 2 = dir, 4 = cwd-dir, 8 = shm
    long len;
    char* path;
    unsigned char* addr;
    StreamMode stream_mode;

}Lattice_FD;
typedef Lattice_FD* LattFD;
typedef Lattice_FD** LattFD_PTP;

typedef union MultiFormMode{
    char c;
    int i;
    char ca[3];
}MultiFormMode;

ConfigMultiTool init_CNFGMT(unsigned int);



#endif