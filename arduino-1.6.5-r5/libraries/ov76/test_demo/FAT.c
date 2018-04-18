/*******************************************************************/
/*          FAT diriver for  MP3 Player                            */
/*                                                                 */
/* Platform   : AVRStudio4.13 b528 + WinAVR20070525                */
/*              optimize -0s                                       */
/* Author     : bozai(Zhang Qibo)                                  */
/* E-mail     : sudazqb@163.com                                    */
/* MSN        : zhangqibo_1985@hotmail.com                         */
/* Date       : 2006-06-14                                         */
/*******************************************************************/
/*2007-11-09: now the songs may placed in any directory, but the   */
/*            total number of folder the card has should less than */
/*            127 for we use 512Bytes EEPROM to stored the info    */
/*2007-10-08: fix a bug (but it never works), so stupid I am,      */
/*            take & as &&                                         */
/*2006-06-14: still the bug of FAT                                 */
/*2006-05-04: fix bug of FAT init, add read capacity check         */

#include"FAT.h"

DWORD FirstDirClust;    //first directory cluster
DWORD FirstDataSector;	// The first sector number of data
WORD BytesPerSector;	// Bytes per sector
WORD FATsectors;		// The amount sector a FAT occupied
WORD SectorsPerClust;	// Sector per cluster
DWORD FirstFATSector;	// The first FAT sector
DWORD FirstDirSector;	// The first Dir sector
DWORD RootDirSectors;	// The sector number a Root dir occupied
DWORD RootDirCount;		// The count of directory in root dir
BYTE FAT32_Enable;

BYTE (* FAT_ReadSector)(DWORD,BYTE *);
BYTE (* FAT_WriteSector)(DWORD,BYTE *);

//函数指针指向sd卡的读写函数
//function pointer to the sd card read & write single block
//wirte sector are not use in this player
BYTE (* FAT_ReadSector)(DWORD sector, BYTE * buffer)=MMC_SD_ReadSingleBlock;//device read
BYTE (* FAT_WriteSector)(DWORD sector, BYTE * buffer)=MMC_SD_WriteSingleBlock;//device write

struct FileInfoStruct FileInfo;//temporarily buffer for file information

//FAT初始化，不含SD的初始化，用之前应先调用sd的初始化
unsigned char FAT_Init(void)//Initialize of FAT  need initialize SD first
{
    struct bootsector710 *bs  = 0;
    struct bpb710        *bpb = 0;
//	struct partsector    *ps  = 0;
    struct partrecord    *pr  = 0;

    BYTE buffer[512];
    DWORD hidsec=0;
    DWORD Capacity;

    Capacity = MMC_SD_ReadCapacity();
    if(Capacity<0xff)return 1;


    if(FAT_ReadSector(0,buffer))return 1;
    bs = (struct bootsector710 *)buffer;
    pr = (struct partrecord *)((struct partsector *)buffer)->psPart;//first partition
    hidsec = pr->prStartLBA;//the hidden sectors
    if(hidsec >= Capacity/512)
    {
        hidsec = 0;
    }
    else
    {
        if(FAT_ReadSector(pr->prStartLBA,buffer))return 1;//read the bpb sector
        bs = (struct bootsector710 *)buffer;
        if(bs->bsJump[0]!=0xE9 && bs->bsJump[0]!=0xEB)
        {
            hidsec = 0;
            if(FAT_ReadSector(0,buffer))return 1;//read the bpb sector
            bs = (struct bootsector710 *)buffer;
        }
    }

    if(bs->bsJump[0]!=0xE9 && bs->bsJump[0]!=0xEB)//对付没有bootsect的sd卡	//dead with the card which has no bootsect
    {
        return 1;
    }

//	GUI_sprintf_chartohex(100,20,bs->bsJump[0],0xffff,0x0000) ;  ////----------------------------------------------
///	GUI_sprintf_chartohex(100,40,bs->bsJump[1],0xffff,0x0000) ;
//	GUI_sprintf_chartohex(100,60,bs->bsJump[2],0xffff,0x0000) ;


    bpb = (struct bpb710 *)bs->bsBPB;


    if(bpb->bpbFATsecs)//detemine thd FAT type  //do not support FAT12
    {
        FAT32_Enable=0;	//FAT16
        FATsectors		= bpb->bpbFATsecs;//FAT占用的扇区数	//the sectors number occupied by one fat talbe
        FirstDirClust = 2;
    }
    else
    {
        FAT32_Enable=1;	//FAT32
        FATsectors		= bpb->bpbBigFATsecs;//FAT占用的扇区数	//the sectors number occupied by one fat talbe
        FirstDirClust = bpb->bpbRootClust;
    }

    BytesPerSector	= bpb->bpbBytesPerSec;//每扇区字节数


//	/I_sprintf_chartobit(10,20,BytesPerSector>>8,0xffff,0x0000) ;  //-----------------------------------------------
//   GUI_sprintf_chartobit(10,40,BytesPerSector,0xffff,0x0000) ;


    SectorsPerClust	= (BYTE)bpb->bpbSecPerClust;//每簇扇区数

//	GUI_sprintf_chartobit(10,60,SectorsPerClust,0xffff,0x0000) ;    //------------------------------------------------

    FirstFATSector	= bpb->bpbResSectors+hidsec;//第一个FAT表扇区
    RootDirCount	= bpb->bpbRootDirEnts;//根目录项数
    RootDirSectors	= (RootDirCount*32)>>9;//根目录占用的扇区数
    FirstDirSector	= FirstFATSector+bpb->bpbFATs*FATsectors;//第一个目录扇区
    FirstDataSector	= FirstDirSector+RootDirSectors;//第一个数据扇区
    return 0;
}

//读一个簇中的一个扇区
//read one sector of one cluster, parameter part indicate which sector
unsigned char FAT_LoadPartCluster(unsigned long cluster,unsigned part,BYTE * buffer)
{
    DWORD sector;
    sector=FirstDataSector+(DWORD)(cluster-2)*(DWORD)SectorsPerClust;//calculate the actual sector number
    if(FAT_ReadSector(sector+part,buffer))return 1;
    else return 0;
}


//读下一簇簇号
//Return the cluster number of next cluster of file
//Suitable for system which has limited RAM
unsigned long FAT_NextCluster(unsigned long cluster)
{
    BYTE buffer[512];
    DWORD sector;
    DWORD offset;
    if(FAT32_Enable)offset = cluster/128;
    else offset = cluster/256;
    if(cluster<2)return 0x0ffffff8;
    sector=FirstFATSector+offset;//calculate the actual sector
    if(FAT_ReadSector(sector,buffer))return 0x0ffffff8;//read fat table / return 0xfff8 when error occured

    if(FAT32_Enable)
    {
        offset=cluster%128;//find the position
        sector=((unsigned long *)buffer)[offset];
    }
    else
    {
        offset=cluster%256;//find the position
        sector=((unsigned int *)buffer)[offset];
    }
    return (unsigned long)sector;//return the cluste number
}

#if FIX_DIRECTORY
//在给定目录下查找文件
//Find a item in the directory which specify by the parameter "cluster"
//Return the start cluster number
unsigned int FAT_FindItem(unsigned long cluster, BYTE *name, struct FileInfoStruct *FileInfo)
{
    BYTE *buffer;
    DWORD tempclust;
    DWORD sector;
    unsigned char cnt;
    unsigned int offset;
    unsigned char i;
    struct direntry *item = 0;
    if((cluster==0) && (FAT32_Enable == 0))// root directory
    {
        buffer=malloc(512);//apply memory
        if(buffer==0)return 1;//if failed
        for(cnt=0; cnt<RootDirSectors; cnt++)
        {
            if(FAT_ReadSector(FirstDirSector+cnt,buffer))
            {
                free(buffer);
                return 1;
            }
            for(offset=0; offset<512; offset+=32)
            {
                item=(struct direntry *)(&buffer[offset]);
                if((item->deName[0] != 0x00) && (item->deName[0] != 0xe5) && (item->deAttributes != 0x0f))
                {
                    for(i=0; i<11; i++)
                    {
                        if(buffer[offset+i]!=name[i])break;
                    }
                    if(i==11)
                    {
                        //return the parameter of the item
                        FileInfo->StartCluster = item->deStartCluster + (((unsigned long)item->deHighClust)<<16);//don't care
                        FileInfo->Size         = item->deFileSize;
                        FileInfo->Attr         = item->deAttributes;
                        FileInfo->Sector       = FirstDirSector+cnt;
                        FileInfo->Offset       = offset;
                        free(buffer);
                        return 0;
                    }
                }
            }
        }
        free(buffer);//release
    }
    else//other folders
    {
        tempclust=cluster;
        while(1)
        {
            sector=FirstDataSector+(DWORD)(tempclust-2)*(DWORD)SectorsPerClust;//calculate the actual sector number
            buffer=malloc(512);//apply memory
            if(buffer==0)return 1;//if failed
            for(cnt=0; cnt<SectorsPerClust; cnt++)
            {
                if(FAT_ReadSector(sector+cnt,buffer))
                {
                    free(buffer);
                    return 1;
                }
                for(offset=0; offset<512; offset+=32)
                {
                    item=(struct direntry *)(&buffer[offset]);
                    if((item->deName[0] != 0x00) && (item->deName[0] != 0xe5) && (item->deAttributes != 0x0f))
                    {
                        for(i=0; i<11; i++)
                        {
                            if(buffer[offset+i]!=name[i])break;
                        }
                        if(i==11)
                        {
                            FileInfo->StartCluster = item->deStartCluster + (((unsigned long)item->deHighClust)<<16);//don't care
                            FileInfo->Size         = item->deFileSize;
                            FileInfo->Attr         = item->deAttributes;
                            FileInfo->Sector       = sector+cnt;
                            FileInfo->Offset       = offset;
                            free(buffer);
                            return 0;
                        }
                    }
                }
            }
            free(buffer);//release
            tempclust=FAT_NextCluster(tempclust);//next cluster
            if(tempclust == 0x0fffffff || tempclust == 0x0ffffff8 || (FAT32_Enable == 0 && tempclust == 0xffff))break;
        }
    }
    return 1;
}

// find a directory with the given path
unsigned long FAT_OpenDir(BYTE * dir)
{
    BYTE name[11];
    BYTE *p=dir;
    BYTE deep=0;
    BYTE i,j;
    DWORD cluster=0;
    if(FAT32_Enable)cluster = FirstDirClust;
    if(*p != '\\')return 1;//invalid path
    while(*p)
    {
        if(*p == '\\')
        {
            deep++;
        }
        p++;
    }
    p=dir;
    for(i=0; i<deep-1; i++)
    {
        p++;
        for(j=0; j<11; j++)name[j]=0x20;
        j=0;
        while(*p != '\\')
        {
            if((*p) >= 'a' && (*p) <= 'z')name[j] = (*p++)-0x20;
            else name[j] = *p++;
            j++;
        }
        if(FAT_FindItem(cluster,name, &FileInfo))return 1;//find the directory
        cluster = FileInfo.StartCluster;
    }
    p++;
    for(j=0; j<11; j++)name[j]=0x20;
    j=0;
    while(*p)
    {
        if(*p>='a' && *p<='z')name[j]=(*p++)-0x20;
        else name[j]=*p++;
        j++;
    }
    if(j == 0)return cluster;
    if(FAT_FindItem(cluster,name, &FileInfo))return 1;//find the final directory
    cluster = FileInfo.StartCluster;
    return cluster;
}
#endif

//复制记录项信息  //copy item
void CopyDirentruyItem(struct direntry *Desti,struct direntry *Source)
{
    BYTE i;
    for(i=0; i<8; i++)Desti->deName[i] = Source->deName[i];
    for(i=0; i<3; i++)Desti->deExtension[i] = Source->deExtension[i];
    Desti->deAttributes = Source->deAttributes;
    Desti->deLowerCase = Source->deLowerCase;
    Desti->deCHundredth = Source->deCHundredth;
    for(i=0; i<2; i++)Desti->deCTime[i] = Source->deCTime[i];
    for(i=0; i<2; i++)Desti->deCDate[i] = Source->deCDate[i];
    for(i=0; i<2; i++)Desti->deADate[i] = Source->deADate[i];
    Desti->deHighClust = Source->deHighClust;
    for(i=0; i<2; i++)Desti->deMTime[i] = Source->deMTime[i];
    for(i=0; i<2; i++)Desti->deMDate[i] = Source->deMDate[i];
    Desti->deStartCluster = Source->deStartCluster;
    Desti->deFileSize = Source->deFileSize;
}

#if FIX_DIRECTORY

//search the file , when *count = 0 it will bring the number whole songs, when *cout != 0 the *MusicInfo will bring the infomation of the file
BYTE Search(BYTE *dir,struct direntry *MusicInfo,WORD *Count,BYTE *type)//当COUNT为零时，有它带回这个目录下总共有多少首音乐
{
    //不为零时有MusicInfo带回第Count首歌的详细文件信息
    BYTE *buffer;
    //BYTE buff[3];
    DWORD sector;
    DWORD cluster;
    DWORD tempclust;
    unsigned char cnt;
    unsigned int offset;
    unsigned int i=0;
    struct direntry *item = 0;
    cluster = FAT_OpenDir(dir);
    if(cluster == 1)return 1;
    if(cluster==0 && FAT32_Enable==0)// root directory
    {
        buffer=malloc(512);//apply memory
        if(buffer==0)return 1;//if failed
        for(cnt=0; cnt<RootDirSectors; cnt++)
        {
            if(FAT_ReadSector(FirstDirSector+cnt,buffer))
            {
                free(buffer);
                return 1;
            }
            for(offset=0; offset<512; offset+=32)
            {
                item=(struct direntry *)(&buffer[offset]);//pointer convert
                //find a valid item and display it
                if((item->deName[0] != '.') && (item->deName[0] != 0x00) && (item->deName[0] != 0xe5) && (item->deAttributes != 0x0f))
                {
                    if((item->deExtension[0] == 'b')&&(item->deExtension[1] == 'm')&&(item->deExtension[2] == 'p'))
                    {
                        CopyDirentruyItem(MusicInfo,item);
                        *type=1;
                        i++;
                        if(i==*Count)
                        {
                            free(buffer);
                            return 0;
                        }
                    }
                }
            }
        }
        free(buffer);//release
    }
    else//other folders
    {
        tempclust=cluster;
        while(1)
        {
            sector=FirstDataSector+(DWORD)(tempclust-2)*(DWORD)SectorsPerClust;//calculate the actual sector number
            buffer=malloc(512);//apply memory
            if(buffer==0)return 1;//if failed
            for(cnt=0; cnt<SectorsPerClust; cnt++)
            {
                if(FAT_ReadSector(sector+cnt,buffer))
                {
                    free(buffer);
                    return 1;
                }
                for(offset=0; offset<512; offset+=32)
                {
                    item=(struct direntry *)(&buffer[offset]);
                    if((item->deName[0] != '.') && (item->deName[0] != 0x00) && (item->deName[0] != 0xe5) && (item->deAttributes != 0x0f))
                    {
                        if((item->deExtension[0] == 'b')&&(item->deExtension[1] == 'm')&&(item->deExtension[2] == 'p'))
                        {
                            CopyDirentruyItem(MusicInfo,item);
                            *type=1;
                            i++;
                            if(i==*Count)
                            {
                                free(buffer);
                                return 0;
                            }
                        }
                    }
                }
            }
            free(buffer);//release
            tempclust=FAT_NextCluster(tempclust);//next cluster
            if(tempclust == 0x0fffffff || tempclust == 0x0ffffff8 || (FAT32_Enable == 0 && tempclust == 0xffff))break;
        }
    }
    if(*Count==0)*Count=i;
    return 0;
}

#else

void WriteFolderCluster(WORD addr,DWORD cluster)
{
#if FAT_DEBUG
    printf_P(PSTR("\r\nWrite EEPROM address:%d with value:%ld"),addr,cluster);
#endif
    eeprom_write_byte( (uint8_t*)addr,cluster>>24);
    eeprom_write_byte( (uint8_t*)addr+1,cluster>>16);
    eeprom_write_byte( (uint8_t*)addr+2,cluster>>8);
    eeprom_write_byte( (uint8_t*)addr+3,cluster>>0);
}

DWORD GetFolderCluster(WORD addr)
{
    DWORD temp;
    temp = eeprom_read_byte( (uint8_t *) addr);
    temp <<= 8;
    temp += eeprom_read_byte( (uint8_t *)(addr+1) );
    temp <<= 8;
    temp += eeprom_read_byte( (uint8_t *)(addr+2) );
    temp <<= 8;
    temp += eeprom_read_byte( (uint8_t *)(addr+3) );
#if FAT_DEBUG
    printf_P(PSTR("\r\nRead EEPROM address: %d value is: %ld"),addr,temp);
#endif
    return temp;
}

BYTE SearchFolder(DWORD cluster,WORD *addr)
{
    BYTE *buffer;
    //BYTE buff[3];
    DWORD sector;
    //DWORD cluster;
    DWORD tempclust;
    unsigned char cnt;
    unsigned int offset;
    //unsigned int i=0;
    //unsigned char j;//long name buffer offset;
//	unsigned char *p;//long name buffer pointer
    struct direntry *item = 0;
    //struct winentry *we =0;

    if(cluster==0 && FAT32_Enable==0)// root directory
    {
        buffer=malloc(512);//apply memory
        if(buffer==0)return 1;//if failed
        for(cnt=0; cnt<RootDirSectors; cnt++)
        {
            if(FAT_ReadSector(FirstDirSector+cnt,buffer))
            {
                free(buffer);
                return 1;
            }
            for(offset=0; offset<512; offset+=32)
            {
                item=(struct direntry *)(&buffer[offset]);//pointer convert
                //find a valid item and display it
                if((item->deName[0] != '.') && (item->deName[0] != 0x00) && (item->deName[0] != 0xe5))
                {
                    if(item->deAttributes & ATTR_DIRECTORY )
                    {
#if FAT_DEBUG
                        printf_P(PSTR("\r\nFound a folder!"));
#endif
                        if(*addr==RECORD_ADDR_END)return 0;
                        else
                        {
                            WriteFolderCluster(*addr,item->deStartCluster+(((unsigned long)item->deHighClust)<<16));
                            *addr+=4;
                        }
                    }
                }
            }
        }
        free(buffer);//release
    }
    else//other folders
    {
        tempclust=cluster;
        while(1)
        {
            sector=FirstDataSector+(DWORD)(tempclust-2)*(DWORD)SectorsPerClust;//calculate the actual sector number
            buffer=malloc(512);//apply memory
            if(buffer==0)return 1;//if failed
            for(cnt=0; cnt<SectorsPerClust; cnt++)
            {
                if(FAT_ReadSector(sector+cnt,buffer))
                {
                    free(buffer);
                    return 1;
                }
                for(offset=0; offset<512; offset+=32)
                {
                    item=(struct direntry *)(&buffer[offset]);
                    if((item->deName[0] != '.') && (item->deName[0] != 0x00) && (item->deName[0] != 0xe5))
                    {
                        if(item->deAttributes & ATTR_DIRECTORY )
                        {
#if FAT_DEBUG
                            printf_P(PSTR("\r\nFound a folder!"));
#endif
                            if(*addr==RECORD_ADDR_END)return 0;
                            else
                            {
                                WriteFolderCluster(*addr,item->deStartCluster+(((unsigned long)item->deHighClust)<<16));
                                *addr+=4;
                            }
                        }
                    }
                }
            }
            free(buffer);//release
            tempclust=FAT_NextCluster(tempclust);//next cluster
            if(tempclust == 0x0fffffff || tempclust == 0x0ffffff8 || (FAT32_Enable == 0 && tempclust == 0xffff))break;
        }
    }
    return 0;
}



BYTE SearchInit(void)
{
    WORD addr = RECORD_ADDR_START;
    WORD temp_addr;
    DWORD cluster;

#if FAT_DEBUG
    printf_P(PSTR("\r\nSearchInit"));
#endif

    if(FAT32_Enable)
        WriteFolderCluster(addr,FirstDirClust);
    else
        WriteFolderCluster(RECORD_ADDR_START,0);

    addr += 4;

    WriteFolderCluster(addr, 0xffffffff);
    temp_addr = addr;
    addr = RECORD_ADDR_START;
    while(1)
    {
        cluster = GetFolderCluster(addr);
        if(cluster == 0xffffffff)return 0;
        else
        {
            if(SearchFolder(cluster,&temp_addr))
            {
#if FAT_DEBUG
                printf_P(PSTR("\r\nERROR: search folder error"));
#endif
            }
#if FAT_DEBUG
            printf_P(PSTR("\r\nSearch one folder, write the end flag if the the value is not the right value"));
#endif
            if(GetFolderCluster(temp_addr) != 0xffffffff)
                WriteFolderCluster(temp_addr,0XFFFFFFFF);
            if(temp_addr == RECORD_ADDR_END)
            {
#if FAT_DEBUG
                printf_P(PSTR("\r\nWARNING: EEPROM is full, no more space!"));
#endif
                WriteFolderCluster(temp_addr - 4,0XFFFFFFFF);
                break;
            }
        }
        addr+=4;
    }
#if FAT_DEBUG
    printf_P(PSTR("\r\nSearch Completed!"));
#endif
    return 0;
}


//search the file , when *count = 0 it will bring the number whole songs, when *cout != 0 the *MusicInfo will bring the infomation of the file
BYTE Search(/*BYTE *dirWORD *music_record_addr,*/struct direntry *MusicInfo,WORD *Count,BYTE *type)//当COUNT为零时，有它带回这个目录下总共有多少首音乐
{
    //不为零时有MusicInfo带回第Count首歌的详细文件信息
    BYTE *buffer;
    DWORD sector;
    DWORD cluster;
    DWORD tempclust;
    unsigned char cnt;
    unsigned int offset;
    unsigned int i=0;
    //unsigned char j;//long name buffer offset;
    //unsigned char *p;//long name buffer pointer
    struct direntry *item = 0;
    //struct winentry *we =0;
    //cluster = FAT_OpenDir(dir);
    //if(cluster == 1)return 1;

    BYTE file_type[3] = {0,0,0} ;

    switch(*type)
    {
    case  1 :
        file_type[0] = 'B' ;
        file_type[1] = 'M' ;
        file_type[2] = 'P' ;

        break ;

    case  2 :
        file_type[0] = 'B' ;
        file_type[1] = 'I' ;
        file_type[2] = 'N' ;

        break;


    }


    WORD addr = RECORD_ADDR_START;
    while(1)
    {
        cluster = GetFolderCluster(addr);
        addr += 4;
        if(cluster == 0xffffffff) break;

        else
        {
            //*music_record_addr = addr - 4;	/* record in which record found the right file */
            if(cluster==0 && FAT32_Enable==0)// root directory
            {


                buffer=malloc(512);//apply memory
                if(buffer==0)return 1;//if failed


                for(cnt=0; cnt<RootDirSectors; cnt++)
                {
                    if(FAT_ReadSector(FirstDirSector+cnt,buffer))
                    {
                        free(buffer);
                        return 1;
                    }
                    for(offset=0; offset<512; offset+=32)
                    {
                        item=(struct direntry *)(&buffer[offset]);//pointer convert
                        //find a valid item and display it
                        if((item->deName[0] != '.') && (item->deName[0] != 0x00) && (item->deName[0] != 0xe5) && (item->deAttributes != 0x0f))
                        {

                            if((item->deExtension[0] == file_type[0])&&(item->deExtension[1] == file_type[1])&&(item->deExtension[2] == file_type[2]))
                            {

                                CopyDirentruyItem(MusicInfo,item);

                                i++;
                                if(i==*Count)
                                {
                                    free(buffer);
                                    return 0;
                                }
                            }
                        }
                    }
                }
                free(buffer);//release
            }
            else//other folders
            {
                tempclust=cluster;
                while(1)
                {
                    sector=FirstDataSector+(DWORD)(tempclust-2)*(DWORD)SectorsPerClust;//calculate the actual sector number
                    buffer=malloc(512);//apply memory
                    if(buffer==0)return 1;//if failed
                    for(cnt=0; cnt<SectorsPerClust; cnt++)
                    {
                        if(FAT_ReadSector(sector+cnt,buffer))
                        {
                            free(buffer);
                            return 1;
                        }
                        for(offset=0; offset<512; offset+=32)
                        {
                            item=(struct direntry *)(&buffer[offset]);
                            if((item->deName[0] != '.') && (item->deName[0] != 0x00) && (item->deName[0] != 0xe5) && (item->deAttributes != 0x0f))
                            {

                                if((item->deExtension[0] == file_type[0])&&(item->deExtension[1] == file_type[1])&&(item->deExtension[2] == file_type[2]))
                                {

                                    CopyDirentruyItem(MusicInfo,item);

                                    i++;
                                    if(i==*Count)
                                    {
                                        free(buffer);
                                        return 0;
                                    }
                                }
                            }
                        }
                    }
                    free(buffer);//release
                    tempclust=FAT_NextCluster(tempclust);//next cluster
                    if(tempclust == 0x0fffffff || tempclust == 0x0ffffff8 || (FAT32_Enable == 0 && tempclust == 0xffff))break;
                }
            }
        }
    }
    if(*Count==0)*Count=i;
    /*
    GUI_sprintf_chartohex(130,300,*type,0xffff,0x0000) ;
    GUI_sprintf_chartohex(160,300,i,0xffff,0x0000) ;
    GUI_sprintf_chartohex(190,300,*Count,0xffff,0x0000) ;
    while(1) ;*/
    return 0;
}
#endif
