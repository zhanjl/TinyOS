/*该程序用来操作bootblock文件，
 *把bootblock文件的大小设为512字节，
 *并且最后两个字节的值为0x55AA。
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
    struct stat buf;
    FILE *fp;
    char temp; 
    fp = fopen(argv[1], "r+");
   
    fstat(fileno(fp), &buf);
    int size = buf.st_size;
    if (size > 510)
    {
        printf("%s is too large\n", argv[1]);
        return -1;
    } 
    fseek(fp, 0, SEEK_END);
    temp = 'a';
    while (size < 510)
    {
        fwrite(&temp, 1, 1, fp);
        size++;
    }
    temp = 0x55;
    fwrite(&temp, 1, 1, fp);
    temp = 0xAA;
    fwrite(&temp, 1, 1, fp);

    fclose(fp);
    return 0;
}
