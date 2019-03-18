

#define ERR_SUCCESS 		                  0
#define ERR_SOURCE_DIR_DOES_NOT_EXIST 		-1
#define ERR_ARG_IS_NOT_A_DIRECTORY				-2
#define ERR_FILE_ALREADY_EXISTS 				-3
#define ERR_FAILED_TO_CREATE_DIR				-4
#define ERR_FILE_DOES_NOT_EXIST 				-5
#define ERR_HARDLINKS_UNSUPPORTED 				-6
#define ERR_CREATE_HARDLINK_FAILED				-7
#define ERR_LESS_CMD_ARGUMENTS					-8
#define ERR_FAILED_TO_ENUMERATE_FILES			-9
#define ERR_TOO_MANY_LINKS					    -10  // Not yet in html docu
#define ERR_NOT_ON_SAME_VOLUME					-11  // Not yet in html docu
#define ERR_SMARTCOPY_FAILED					-12
#define ERR_NO_HARDLINKGROUPS					-13  // Not yet in html docu
#define ERR_SMARTCLONE_FAILED         -14


enum { 
    SET_WRITETIME = 1,
    SET_ACCESSTIME = 2,
    SET_CREATIONTIME = 4,
    SET_BACKUP = 8,
    SET_CTIME = 16,
    SET_PRINT_FILENAME = 32
};


