#include <stdlib.h>

#include "const.h"
#include "transplant.h"
#include "debug.h"


#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

int mystringcompare (const char *str1, const char* str2);
/*
 * You may modify this file and/or move the functions contained here
 * to other source files (except for main.c) as you wish.
 *
 * IMPORTANT: You MAY NOT use any array brackets (i.e. [ and ]) and
 * you MAY NOT declare any arrays or allocate any storage with malloc().
 * The purpose of this restriction is to force you to use pointers.
 * Variables to hold the pathname of the current file or directory
 * as well as other data have been pre-declared for you in const.h.
 * You must use those variables, rather than declaring your own.
 * IF YOU VIOLATE THIS RESTRICTION, YOU WILL GET A ZERO!
 *
 * IMPORTANT: You MAY NOT use floating point arithmetic or declare
 * any "float" or "double" variables.  IF YOU VIOLATE THIS RESTRICTION,
 * YOU WILL GET A ZERO!
 */

/*
 * A function that returns printable names for the record types, for use in
 * generating debugging printout.
 */
static char *record_type_name(int i) {
    switch(i) {
        case START_OF_TRANSMISSION:
        return "START_OF_TRANSMISSION";
        case END_OF_TRANSMISSION:
        return "END_OF_TRANSMISSION";
        case START_OF_DIRECTORY:
        return "START_OF_DIRECTORY";
        case END_OF_DIRECTORY:
        return "END_OF_DIRECTORY";
        case DIRECTORY_ENTRY:
        return "DIRECTORY_ENTRY";
        case FILE_DATA:
        return "FILE_DATA";
        default:
        return "UNKNOWN";
    }
}

/*
 * @brief  Initialize path_buf to a specified base path.
 * @details  This function copies its null-terminated argument string into
 * path_buf, including its terminating null byte.
 * The function fails if the argument string, including the terminating
 * null byte, is longer than the size of path_buf.  The path_length variable
 * is set to the length of the string in path_buf, not including the terminating
 * null byte.
 *
 * @param  Pathname to be copied into path_buf.
 * @return 0 on success, -1 in case of error
 */
int path_init(char *name) {
    char *temporary = name;
    int length = 0;
    int l1 =0;

    while (*temporary != '\0'){
        length++;
        temporary++;
    }

    if(PATH_MAX < length+1){
        return -1;
    }

    path_length = length;
    char *pathbuf_pointer = path_buf;

    while(*name){
        *pathbuf_pointer = *name;
        pathbuf_pointer++;
        name++;
    }

    *pathbuf_pointer = '\0';

    return 0;
}

/*
 * @brief  Append an additional component to the end of the pathname in path_buf.
 * @details  This function assumes that path_buf has been initialized to a valid
 * string.  It appends to the existing string the path separator character '/',
 * followed by the string given as argument, including its terminating null byte.
 * The length of the new string, including the terminating null byte, must be
 * no more than the size of path_buf.  The variable path_length is updated to
 * remain consistent with the length of the string in path_buf.
 *
 * @param  The string to be appended to the path in path_buf.  The string must
 * not contain any occurrences of the path separator character '/'.
 * @return 0 in case of success, -1 otherwise.
 */
int path_push(char *name) {

    char *pathbuf_pointer = path_buf;
    char *name_copy = name;
    int lengthofnamecopy = 0;

    while(*name_copy){

        if(*name_copy == '/')
        {
            return -1;
        }

        name_copy++;
        lengthofnamecopy++;
    }

    if((path_length + lengthofnamecopy) > PATH_MAX){

        return -1;
    }

    while(*pathbuf_pointer){
        pathbuf_pointer++;
    }

    *pathbuf_pointer = '/';
    pathbuf_pointer++;

    name_copy = name;

    while(*name_copy)
    {
        *pathbuf_pointer = *name_copy;
        name_copy++;
        pathbuf_pointer++;
    }

    path_length+=lengthofnamecopy;
    *pathbuf_pointer = '\0';

    return -1;
}

/*
 * @brief  Remove the last component from the end of the pathname.
 * @details  This function assumes that path_buf contains a non-empty string.
 * It removes the suffix of this string that starts at the last occurrence
 * of the path separator character '/'.  If there is no such occurrence,
 * then the entire string is removed, leaving an empty string in path_buf.
 * The variable path_length is updated to remain consistent with the length
 * of the string in path_buf.  The function fails if path_buf is originally
 * empty, so that there is no path component to be removed.
 *
 * @return 0 in case of success, -1 otherwise.
 */
int path_pop() {

    char *pathtopop = path_buf;
    pathtopop+= path_length;

    while(*pathtopop != '/' || path_length == 0)
    {
        *pathtopop = '\0';
        pathtopop-=1;
        path_length--;
    }

    if(path_length!= 0)
    {
        *pathtopop = '\0';
        path_length--;
    }
    return 0;
}

/*
 * @brief  Serialize the contents of a directory as a sequence of records written
 * to the standard output.
 * @details  This function assumes that path_buf contains the name of an existing
 * directory to be serialized.  It serializes the contents of that directory as a
 * sequence of records that begins with a START_OF_DIRECTORY record, ends with an
 * END_OF_DIRECTORY record, and with the intervening records all of type DIRECTORY_ENTRY.
 *
 * @param depth  The value of the depth field that is expected to occur in the
 * START_OF_DIRECTORY, DIRECTORY_ENTRY, and END_OF_DIRECTORY records processed.
 * Note that this depth pertains only to the "top-level" records in the sequence:
 * DIRECTORY_ENTRY records may be recursively followed by similar sequence of
 * records describing sub-directories at a greater depth.
 * @return 0 in case of success, -1 otherwise.  A variety of errors can occur,
 * including failure to open files, failure to traverse directories, and I/O errors
 * that occur while reading file content and writing to standard output.
 */
int serialize_directory(int depth) {
    //debug("i m in serialize_directory");
    struct dirent* direntobject;
    struct stat statbuff;
    DIR * directory = opendir(path_buf);
    int type = 0;

    if(directory == NULL)
        return -1;

    while((direntobject = readdir(directory)) != NULL)
    {
        path_push(direntobject->d_name);
        char *temporary = direntobject->d_name;
        int lengthoffile = 0;

        while(*temporary)
        {
            lengthoffile++;
            temporary+=1;
        }

        temporary = direntobject->d_name;
        stat(path_buf, &statbuff);

        if((*(direntobject->d_name) == '.') || (*(direntobject->d_name+1) == '.'))
        {
            path_pop();
        }
        else
        {
            if(S_ISDIR(statbuff.st_mode))
            {

                int magicByte_1 = 0x0c;
                int magicByte_2 = 0x0d;
                int magicByte_3 = 0xed;
                int magicByte_4 = 4;

                putchar(magicByte_1);
                putchar(magicByte_2);
                putchar(magicByte_3);
                putchar(magicByte_4);

                int i =3;

                while(i>-1)
                {
                    putchar((depth)>> (i*8)& 0xff);
                    i--;
                }

                i =7;
                long random = 28 + lengthoffile;

                while(i>-1)
                {
                    putchar((random)>> (i*8)& 0xff);
                    i--;
                }

                i = 3;
                while(i>-1)
                {
                    putchar((statbuff.st_mode) >> (i*8)& 0xff);
                    i--;
                }

                i =7;

                while(i>-1)
                {
                    putchar((statbuff.st_size) >> (i*8)& 0xff);
                    i--;
                }

                while(*temporary)
                {
                    putchar(*temporary);
                    temporary+=1;
                }

                depth++;

                magicByte_1 = 0x0c;
                magicByte_2 = 0x0d;
                magicByte_3 = 0xed;
                magicByte_4 = 2;

                putchar(magicByte_1);
                putchar(magicByte_2);
                putchar(magicByte_3);
                putchar(magicByte_4);

                i =3;

                while(i>-1)
                {
                    putchar((depth)>> (i*8)& 0xff);
                    i--;
                }

                i =7;
                random = 16;

                while(i>-1)
                {
                    putchar((random)>> (i*8)& 0xff);
                    i--;
                }


                //fprintf(stdout, "0c 0d ed %02x %08x %016x\n", 2, depth, 16);
                serialize_directory(depth);

                magicByte_1 = 0x0c;
                magicByte_2 = 0x0d;
                magicByte_3 = 0xed;
                magicByte_4 = 3;

                putchar(magicByte_1);
                putchar(magicByte_2);
                putchar(magicByte_3);
                putchar(magicByte_4);

                i = 3;

                while(i>-1)
                {
                    putchar((depth)>> (i*8)& 0xff);
                    i--;
                }

                i =7;
                random = 16;

                while(i>-1)
                {
                    putchar((random)>> (i*8)& 0xff);
                    i--;
                }
                //fprintf(stdout, "0c 0d ed %02x %08x %016x\n", 3, depth, 16);
            }

            if(S_ISREG(statbuff.st_mode))
            {

               int magicByte_1 = 0x0c;
               int magicByte_2 = 0x0d;
               int magicByte_3 = 0xed;
               int magicByte_4 = 4;

               putchar(magicByte_1);
               putchar(magicByte_2);
               putchar(magicByte_3);
               putchar(magicByte_4);

               int i = 3;

               while(i>-1)
               {
                putchar((depth)>> (i*8)& 0xff);
                i--;
            }

            i = 7;
            long random = 28 + lengthoffile;

            while(i>-1)
            {
                putchar((random)>> (i*8)& 0xff);
                i--;
            }

            i =3;

            while(i>-1)
            {
                putchar((statbuff.st_mode) >> (i*8)& 0xff);
                i--;
            }

            i =7;

            while(i>-1)
            {
                putchar((statbuff.st_size) >> (i*8)& 0xff);
                i--;
            }
            //fprintf(stdout, "0c 0d ed %02x %08x %016x %08x %016lx", 4, depth, 28 + lengthoffile, statbuff.st_mode, statbuff.st_size);
            while(*temporary)
            {
                putchar(*temporary);
                //fprintf(stdout, "%02x", *temporary);
                temporary+=1;
            }
            serialize_file(depth, statbuff.st_size);
            path_pop();
        }
    }
}
closedir(directory);

return 0;
}

/*
 * @brief  Serialize the contents of a file as a single record written to the
 * standard output.
 * @details  This function assumes that path_buf contains the name of an existing
 * file to be serialized.  It serializes the contents of that file as a single
 * FILE_DATA record emitted to the standard output.
 *
 * @param depth  The value to be used in the depth field of the FILE_DATA record.
 * @param size  The number of bytes of data in the file to be serialized.
 * @return 0 in case of success, -1 otherwise.  A variety of errors can occur,
 * including failure to open the file, too many or not enough data bytes read
 * from the file, and I/O errors reading the file data or writing to standard output.
 */
int serialize_file(int depth, off_t size) {
    FILE *filetoopen;
    char c;
    filetoopen = fopen(path_buf, "r");

    int magicByte_1 = 0x0c;
    int magicByte_2 = 0x0d;
    int magicByte_3 = 0xed;
    int magicByte_4 = 5;

    putchar(magicByte_1);
    putchar(magicByte_2);
    putchar(magicByte_3);
    putchar(magicByte_4);

    int i =3;

    while(i>-1)
    {
        putchar((depth)>> (i*8)& 0xff);
        i--;
    }

    i =7;
    long random = size+16;

    while(i>-1)
    {
        putchar((random)>> (i*8)& 0xff);
        i--;
    }

    while(1)
    {
        c = fgetc(filetoopen);
        if(c == EOF)
            break;
        putchar(c);
    }

    ////(stdout, "0c 0e ed %02x %08x %016lx", 5, depth, size+16);


    fclose(filetoopen);

    return 0;

}

/**
 * @brief Serializes a tree of files and directories, writes
 * serialized data to standard output.
 * @details This function assumes path_buf has been initialized with the pathname
 * of a directory whose contents are to be serialized.  It traverses the tree of
 * files and directories contained in this directory (not including the directory
 * itself) and it emits on the standard output a sequence of bytes from which the
 * tree can be reconstructed.  Options that modify the behavior are obtained from
 * the global_options variable.
 *
 * @return 0 if serialization completes without error, -1 if an error occurs.
 */
int serialize() {

    int depth = 1;
    int magicByte_1 = 0x0c;
    int magicByte_2 = 0x0d;
    int magicByte_3 = 0xed;
    int magicByte_4 = 0;

    putchar(magicByte_1);
    putchar(magicByte_2);
    putchar(magicByte_3);
    putchar(magicByte_4);

    int i = 3;
    int zeerro = 0;

    while(i>-1)
    {
        putchar((zeerro)>> (i*8) & 0xff);
        i--;
    }


    i =7;
    long random = 16;

    while(i>-1)
    {
        putchar((random)>> (i*8) & 0xff);
        i--;
    }

 //fprintf(stdout,"0c 0d ed 00 00000000 0000000000000010\n");

    magicByte_4 = 2;

    putchar(magicByte_1);
    putchar(magicByte_2);
    putchar(magicByte_3);
    putchar(magicByte_4);

    i =3;

    while(i>-1)
    {
        putchar((depth)>> (i*8)& 0xff);
        i--;
    }

    i =7;

    random = 16;

    while(i>-1)
    {
        putchar((random)>> (i*8)& 0xff);
        i--;
    }
 //fprintf(stdout, "0c 0d ed %02x %08x %016x\n", 2, depth, 16);

    serialize_directory(depth);


    magicByte_4 = 3;

    putchar(magicByte_1);
    putchar(magicByte_2);
    putchar(magicByte_3);
    putchar(magicByte_4);

    i =3;
    while(i>-1)
    {
        putchar((depth)>> (i*8)& 0xff);
        i--;
    }

    i = 7;
    random = 16;

    while(i>-1)
    {
        putchar((random)>> (i*8)& 0xff);
        i--;
    }
 //fprintf(stdout, "0c 0d ed %02x %08x %016x\n", 3, depth, 16);

    magicByte_4 = 1;

    putchar(magicByte_1);
    putchar(magicByte_2);
    putchar(magicByte_3);
    putchar(magicByte_4);

    i =3;
    //depth--;

    while(i>-1)
    {
        putchar((zeerro)>> (i*8)& 0xff);
        i--;
    }

    i =7;
    random = 16;

    while(i>-1)
    {
        putchar((random)>> (i*8)& 0xff);
        i--;
    }
 //fprintf(stdout,"0c 0d ed 01 00000000 0000000000000010\n");

    return 0;
}

/*
 * @brief Deserialize directory contents into an existing directory.
 * @details  This function assumes that path_buf contains the name of an existing
 * directory.  It reads (from the standard input) a sequence of DIRECTORY_ENTRY
 * records bracketed by a START_OF_DIRECTORY and END_OF_DIRECTORY record at the
 * same depth and it recreates the entries, leaving the deserialized files and
 * directories within the directory named by path_buf.
 *
 * @param depth  The value of the depth field that is expected to be found in
 * each of the records processed.
 * @return 0 in case of success, -1 in case of an error.  A variety of errors
 * can occur, including depth fields in the records read that do not match the
 * expected value, the records to be processed to not being with START_OF_DIRECTORY
 * or end with END_OF_DIRECTORY, or an I/O error occurs either while reading
 * the records from the standard input or in creating deserialized files and
 * directories.
 */
int deserialize_directory(int depth) {

    //debug("-----deserialize_directory-----");

    // Following is mandetory part which  deals with 02....... which means directory start.

    int i       = 3;
    //int depth   = 0;
    int size    = 0;

    //debug("I am inside deserialize_directory: \n");

    int ddd = 0;
    if(0xc == getchar() && 0xd == getchar() && 0xed == getchar())
    {

        int type_1 = getchar();
        //fprintf(stdout, "type: %d ", type_1);

        if (type_1 == 2)
        {
            //fprintf(stdout, "\nI am going to find: 02(SOD)\n" );
            while(i > -1)
            {
                ddd+= getchar() << (i*8);
                i--;
            }

            // if(ddd != depth)
            // {
            //     return -1;
            // }

            i = 7;
            while(i > -1)
            {
                size += (getchar()) << (i*8);
                i--;
            }
        }
        else {
            //debug("outside while");
            return -1;}
        }
        else
        {
        //debug("osw1");
            return -1;
        }

   // fprintf(stdout,"depth: %d ", ddd);
    // fprintf(stdout, "size: %d \n", size);

    // Now i will check for 04....
    //while(  )

        while(0xc == getchar() && 0xd == getchar() && 0xed == getchar())
        {
            //fprintf(stdout, "\nI am on going to find: 04(DE) " );
        //fprintf(stdout," Directory name is: %s ", path_buf);
            int type_1 = getchar();
            //fprintf(stdout, " type: %d \n", type_1);
            if (type_1 == 0x4)
            {
                i = 3;
                int ddd =0;
                while(i > -1)
                {
                    ddd+= getchar() << (i*8);
                    i--;
                }

                if(ddd != depth){
               // debug("ffff");
                    return -1;
                }

                i = 7;
                int size =0;
                while(i > -1)
                {
                    size += (getchar()) << (i*8);
                    i--;
                }

                int typeandpermissions = 0;

                i = 3;
                while(i > -1)
                {
                    typeandpermissions+= getchar() << (i*8);
                    i--;
                }

                int sizeinformation = 0;

                i = 7;
                while(i > -1)
                {
                    sizeinformation += (getchar()) << (i*8);
                    i--;
                }

               // fprintf(stdout,"depth: %d ", depth);
               // fprintf(stdout, "size: %d ", size);
               // fprintf(stdout,"type and permissions: %d ", typeandpermissions);
               // fprintf(stdout, "size information: %d \n\n", sizeinformation);

                int nameof = size - 28;
                char *nameoffile_directory = name_buf;

            //fprintf(stdout, "name length: %d ", nameof);

                while(nameof!=0)
                {
                //fprintf(stdout, "%d ", getchar() );
                    *nameoffile_directory = getchar();
                    nameoffile_directory++;
                    nameof--;
                }
                *nameoffile_directory = '\0';

                nameoffile_directory = name_buf;

                path_push(nameoffile_directory);

                if(S_ISREG(typeandpermissions))
                {
                    deserialize_file(depth);
                //debug("fff");
                    path_pop();
                }
                else
                {
                        mkdir(path_buf,0700);
                   // mkdir(path_buf, 0700);

                    deserialize_directory(depth+1);
                   // fprintf(stdout,"name is: %s\n", path_buf);
                    path_pop();
                }
            }


            else if(type_1 == 0x3)
            {

                int type_3 = getchar();
                //fprintf(stdout, "type: %d ", type_1);

                //fprintf(stdout, "\nI am going to find: 03(SOD)\n" );
                int i=3;
                int ddd =0;
                int size = 0;
                while(i > -1)
                {
                    ddd+= getchar() << (i*8);
                    i--;
                }

            // if(ddd != depth)
            // {
            //     return -1;
            // }

                i = 7;
                while(i > -1)
                {
                    size += (getchar()) << (i*8);
                    i--;
                }

            }
            else
                {//debug("fff");
            return -1;
        }
    }

    return 0;
}

/*
 * @brief Deserialize the contents of a single file.
 * @details  This function assumes that path_buf contains the name of a file
 * to be deserialized.  The file must not already exist, unless the ``clobber''
 * bit is set in the global_options variable.  It reads (from the standard input)
 * a single FILE_DATA record containing the file content and it recreates the file
 * from the content.
 *
 * @param depth  The value of the depth field that is expected to be found in
 * the FILE_DATA record.
 * @return 0 in case of success, -1 in case of an error.  A variety of errors
 * can occur, including a depth field in the FILE_DATA record that does not match
 * the expected value, the record read is not a FILE_DATA record, the file to
 * be created already exists, or an I/O error occurs either while reading
 * the FILE_DATA record from the standard input or while re-creating the
 * deserialized file.
 */
int deserialize_file(int depth)
{

    FILE *filetoopen;
    char c;
    filetoopen = fopen(path_buf, "w");

    if(0xc == getchar() && 0xd == getchar() && 0xed == getchar() && getchar() == 5 )
    {
        //fprintf(stdout, "\nI am on going to find: 05(DE) \n");

        int i = 3;
        int file_size = 0;
        int file_depth = 0;
        int file_data_1 = 0;

        while(i > -1)
        {
            file_depth+= getchar() << (i*8);
            i--;
        }

        //fprintf(stdout, " file_depth: %d ", file_depth);

        i = 7;
        while(i > -1)
        {
            file_size += (getchar()) << (i*8);
            i--;
        }

        //fprintf(stdout, "file_size: %d\n", file_size);

        int loopcounter = file_size - 16;

        //fprintf(stdout, "loop counter: %d\n", loopcounter);

        while(loopcounter!=0)
        {
            c = getchar();
            fputc(c,filetoopen);
            loopcounter--;
        }

        fclose(filetoopen);

    }
    else
    {
        return -1;
    }

    return 0;
}

/**
 * @brief Reads serialized data from the standard input and reconstructs from it
 * a tree of files and directories.
 * @details  This function assumes path_buf has been initialized with the pathname
 * of a directory into which a tree of files and directories is to be placed.
 * If the directory does not already exist, it is created.  The function then reads
 * from from the standard input a sequence of bytes that represent a serialized tree
 * of files and directories in the format written by serialize() and it reconstructs
 * the tree within the specified directory.  Options that modify the behavior are
 * obtained from the global_options variable.
 *
 * @return 0 if deserialization completes without error, -1 if an error occurs.
 */
int deserialize() {

    //debug("%s \n", path_buf );
    //return 0;
    DIR *makeorno = opendir(path_buf);
    if(!makeorno)
    {
        mkdir(path_buf,0700);
    }

    int i = 3;
    int depth = 0;
    int size = 0;


    if(0xc == getchar() && 0xd == getchar() && 0xed == getchar())
    {
        int type_1 = getchar();

        if (type_1 != 0)
            return -1;

        while(i > -1)
        {
            depth+= getchar() << (i*8);
            i--;
        }

        i = 7;
        while(i > -1)
        {
            size += (getchar()) << (i*8);
            i--;
        }
    }
    else
    {
        return -1;
    }
   // fprintf(stdout,"Depth: %d ",depth );
   // fprintf(stdout,"Size: %d\n",size );

    int v = deserialize_directory(1);
    //debug("%d ",v);

    if(0xc == getchar() && 0xd == getchar() && 0xed == getchar())
    {
        int type_1 = getchar();

        if (type_1 != 0)
            return -1;

        while(i > -1)
        {
            depth+= getchar() << (i*8);
            i--;
        }

        i = 7;
        while(i > -1)
        {
            size += (getchar()) << (i*8);
            i--;
        }
    }
    else
    {
        return -1;
    }


    return 0;

}

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the selected program options will be set in the
 * global variable "global_options", where they will be accessible
 * elsewhere in the program.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * Refer to the homework document for the effects of this function on
 * global variables.
 * @modifies global variable "global_options" to contain a bitmap representing
 * the selected options.
 */
int validargs(int argc, char **argv)
{
    argv++;
    char* incoming = *argv+1;
    int hisfound = 0;

    if (argc <2)
        return -1;

    if (*incoming == 'h')
    {
        global_options = global_options | 0x01;
       // debug("hi I found h\n");
        hisfound = 1;
        return 0;
    }
    else if(*incoming == 's')
        global_options = global_options | 0x02;
    else if (*incoming == 'd')
    {
        //debug(" i ")
        global_options = global_options | 0x04;
    }
    else
    {
        global_options = global_options | 0x00;
        return -1;
    }

    if (!hisfound)
    {
        argv++;
        if(*argv == NULL){
         path_init(".");
         return 0;
     }

     incoming = *argv+1;

     while(incoming != NULL)
     {

        if(*incoming == 'p')
        {
            //debug("%d", *incoming);

            //global_options = global_options | 0x02;
            argv++;
            incoming = *argv;


            if(*argv == NULL || *incoming == '-')
                return -1;
            else
            {
                path_init(incoming);
            }

            argv++;
            incoming = *argv;

            if(*argv == NULL)
            {
                return 0;
            }
            else
            {
                incoming = *argv+1;
            }
        }


        //debug("%d", *incoming);

        else if(*incoming == 'c' && global_options == 2)
        {
           // global_options = global_options | 0x08;
            //debug("go: %d",global_options);
            return -1;
        }
        else
        {
            return -1;
        }

    }
}

return 0;
}
