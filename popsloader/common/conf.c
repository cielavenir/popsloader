/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

#include <pspkernel.h>
#include <pspreg.h>
#include <stdio.h>
#include <string.h>
#include <pspsysmem_kernel.h>
#include <pspthreadman_kernel.h>
#include <pspctrl.h>
#include <pspiofilemgr_kernel.h>
#include <psprtc.h>
#include "popsloader.h"
#include "utils.h"
#include "libs.h"
#include "strsafe.h"
#include "systemctrl.h"
#include "main.h"

int is_ef0(void);

typedef struct _CONFIG {
	char disc_id[12];
	u32 version;
} CONFIG;

struct popsloader_config g_conf;

char __disc_id[16];
static int config_offset = -1;

#if 0
static int get_disc_id()
{
	void* (*sceKernelGetGameInfo_k)();

	memset(__disc_id, 0, sizeof(__disc_id));
	sceKernelGetGameInfo_k = (void *)sctrlHENFindFunction("sceSystemMemoryManager", "SysMemForKernel", 0xCD617A94); 

	if(sceKernelGetGameInfo_k != NULL) {
		char *info_buff = sceKernelGetGameInfo_k();
		memcpy(__disc_id, info_buff + 0x44, 9);
	}

	return 0;
}
#endif

///
static unsigned int read32(const void *p){
	const unsigned char *x=(const unsigned char*)p;
	return x[0]|(x[1]<<8)|(x[2]<<16)|(x[3]<<24);
}
static unsigned short read16(const void *p){
	const unsigned char *x=(const unsigned char*)p;
	return x[0]|(x[1]<<8);
}
static char head[16];
//#define Z(S) (S),strlen(S)
static int get_disc_id()
{
	void* (*sceKernelGetGameInfo_k)();
	//SceUID f=sceIoOpen("ms0:/pops.log", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777);

	memset(__disc_id, 0, sizeof(__disc_id));
	sceKernelGetGameInfo_k = (void *)sctrlHENFindFunction("sceSystemMemoryManager", "SysMemForKernel", 0xCD617A94); 

	if(sceKernelGetGameInfo_k != NULL) {
		char *info_buff = sceKernelGetGameInfo_k();
		memcpy(__disc_id, info_buff + 0x44, 9); //seems not working

		// in this point, sceKernelGetGameInfo() doesn't grub Disc ID.
		// instead, I'm going to parse EBOOT.PBP's PARAM.SFO
		// *** This clause can be used under any term (CC0). [popsdeco]
		if(!*__disc_id){
			//sceIoWrite(f,Z("Entering Fallback mode.\n"));
			//sceIoWrite(f,info_buff,0x100);
			SceUID fd=sceIoOpen(info_buff + 0x74,PSP_O_RDONLY,0777);
			sceIoRead(fd,head,16);
			if(memcmp(head,"\0PBP",4)){ //||read32(head+4)!=0x00010000){ //rare case? 0x00010001
				sceIoClose(fd);
				//sceIoWrite(f,Z("PBP magic error.\n"));
				goto end;//return 0;
			}
			int param_offset=read32(head+8);
			int param_size=read32(head+12)-param_offset;
			SceUID uid=sceKernelAllocPartitionMemory(2,"EBOOTReader",PSP_SMEM_Low,param_size,NULL);
			if(uid<0){
				sceIoClose(fd);
				//sceIoWrite(f,Z("malloc failed.\n"));
				goto end;//return 0;
			}
			char *p=sceKernelGetBlockHeadAddr(uid);//malloc(param_size);
			//char p[0x400];
			sceIoLseek(fd,param_offset,SEEK_SET);
			sceIoRead(fd,p,param_size);
			sceIoClose(fd);
			if(memcmp(p,"\0PSF",4)||read32(p+4)!=0x00000101){
				//free(p);
				sceKernelFreePartitionMemory(uid);
				//sceIoWrite(f,Z("PSF magic error.\n"));
				goto end;//return 0;
			}
			int label_offset=read32(p+8);
			int data_offset=read32(p+12);
			int nlabel=read32(p+16);
			int i=0;
			for(;i<nlabel;i++){
				if(!strcmp(p+label_offset+read16(p+20+16*i),"DISC_ID")){
					memcpy(__disc_id,p+data_offset+read32(p+20+16*i+12),9);
					//sceIoWrite(f,Z(__disc_id));
					//sceIoWrite(f,Z("\n"));
					break;
				}
			}
			//fwrite(p,1,param_size,stdout);
			//free(p);
			sceKernelFreePartitionMemory(uid);
		}
		/// parse EBOOT end.
	}
end:
	//sceIoClose(f);
	return 0;
}
///
#if 0
static inline int is_ef0(void)
{
	return psp_model == PSP_GO && sctrlKernelBootFrom() == 0x50 ? 1 : 0;
}
#endif
int save_config(void)
{
	SceUID fd;
	char path[256];
	CONFIG cnf;

	if(!*__disc_id)return 0;

	sprintf(path, "%s%s", is_ef0() ? "ef" : "ms", CFG_PATH);
	fd = sceIoOpen(path, (config_offset >= 0)? PSP_O_RDWR : PSP_O_WRONLY | PSP_O_CREAT | PSP_O_APPEND, 0777);

	if(fd < 0) {
		return fd;
	}

	memset(&cnf, 0, sizeof(cnf));
	memcpy(cnf.disc_id, __disc_id, 12);
	cnf.version = g_conf.pops_fw_version;

	if(config_offset >= 0) {
		sceIoLseek(fd, config_offset * sizeof(CONFIG), PSP_SEEK_SET);
	}else{
		SceIoStat st;
		if(sceIoGetstat(path,&st)>=0)config_offset=st.st_size>>4;
	}

	sceIoWrite(fd, &cnf, sizeof(cnf));
	sceIoClose(fd);

	return 0;
}

static int _load_config(void)
{
	SceUID fd;
	char path[256];
	CONFIG config[32];
	int size;
	int offset = 0;

	sprintf(path, "%s%s", is_ef0() ? "ef" : "ms", CFG_PATH);
	{
		SceIoStat stat;
		if(sceIoGetstat(path,&stat)>=0 && (stat.st_size&0xf)){
			sceIoRemove(path); //need to remove v1 conf
			return -1;
		}
	}
	fd = sceIoOpen(path, PSP_O_RDONLY, 0777);

	if(fd < 0) {
		return fd;
	}

	while((size = sceIoRead(fd, config, sizeof(CONFIG) * 32))  > 0) {
		int cnt = size / sizeof(CONFIG);

		if(cnt > 0) {
			int i;

			for(i=0; i<cnt; i++) {
				if(0 == memcmp(__disc_id, config[i].disc_id, 9)) {		
					sceIoClose(fd);
					g_conf.pops_fw_version = config[i].version;
					config_offset = offset + i;

					return 0;
				}
			}

			offset += cnt;
		}
	}

	sceIoClose(fd);

	return -1;
}

int load_config(void)
{
	int ret;

	get_disc_id();
	ret = _load_config();

	if(ret < 0) {
		def_config(&g_conf);
		save_config();
	}

	return 0;
}

void def_config(struct popsloader_config *conf)
{
	conf->pops_fw_version = 0;
}
