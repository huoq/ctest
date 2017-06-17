/*****************************************************************************/
/** 
* \file       dsa_parse_elf.c
* \author     hejian
* \date       2015/06/09
* \version    FlexBNG V1
* \brief      DSA模块ELF文件符号解析实现
* \note       Copyright (c) 2000-2020  赛特斯信息科技股份有限公司
* \remarks    修改日志
******************************************************************************/
#include "dsa_incl.h"
#include <libelf.h>
#include <gelf.h>

typedef struct tagDSA_ELF_DATA
{
	INT32 fd;
	Elf *elf;
	GElf_Ehdr ehdr;
	Elf_Data *symtab;
	const char *strtab;
	size_t symtab_count;
	GElf_Addr bias;
	GElf_Addr entry_addr;
	GElf_Addr base_addr;
}DSA_ELF_DATA;

DSA_ELF_DATA  g_dsa_elf_data;

#if  DSA_CODE("内部接口")
VOID dsa_elf_read_symbol_table(DSA_ELF_DATA *lte, const char *filename,
		  Elf_Scn *scn, GElf_Shdr *shdr, const char *name,
		  Elf_Data **datap, size_t *countp, const char **strsp)
{
	*datap = elf_getdata(scn, NULL);
	*countp = shdr->sh_size / shdr->sh_entsize;

	scn = elf_getscn(lte->elf, shdr->sh_link);
	GElf_Shdr shdr2;
	if (scn == NULL || gelf_getshdr(scn, &shdr2) == NULL) {
		ds_log("Couldn't get header of section"
			" #%d from \"%s\": %s\n",
			shdr->sh_link, filename, elf_errmsg(-1));
		return;
	}

	Elf_Data *data = elf_getdata(scn, NULL);
	if (data == NULL || elf_getdata(scn, data) != NULL
	    || shdr2.sh_size != data->d_size || data->d_off) {
		ds_log("Couldn't get data of section"
			" #%d from \"%s\": %s\n",
			shdr2.sh_link, filename, elf_errmsg(-1));
		return;
	}

	*strsp = data->d_buf;
}
#endif

INT32  dsa_elf_read_symbols(const CHAR *path_name)
{
    size_t i = 0;
    GElf_Phdr phdr;
    
	g_dsa_elf_data.fd = open(path_name, O_RDONLY);
	if (g_dsa_elf_data.fd == -1)
	{
		ds_log("Can't open %s: %s\n", path_name,
			strerror(errno));
		goto  EXIT_LABEL;
	}

    elf_version(EV_CURRENT);
	g_dsa_elf_data.elf = elf_begin(g_dsa_elf_data.fd, ELF_C_READ_MMAP, NULL);

	if (g_dsa_elf_data.elf == NULL || elf_kind(g_dsa_elf_data.elf) != ELF_K_ELF)
	{
		ds_log("\"%s\" is not an ELF file\n", path_name);
		goto  EXIT_LABEL;
	}

	if (gelf_getehdr(g_dsa_elf_data.elf, &g_dsa_elf_data.ehdr) == NULL)
	{
		ds_log("can't read ELF header of \"%s\": %s\n",
			path_name, elf_errmsg(-1));
		goto  EXIT_LABEL;
	}

	for (i = 0; gelf_getphdr (g_dsa_elf_data.elf, i, &phdr) != NULL; ++i) {
		if (phdr.p_type == PT_LOAD) {
			g_dsa_elf_data.base_addr = phdr.p_vaddr;
			break;
		}
	}

	g_dsa_elf_data.entry_addr = g_dsa_elf_data.ehdr.e_entry + g_dsa_elf_data.bias;

	if (g_dsa_elf_data.base_addr == 0) {
		ds_log(
			"Couldn't determine base address of %s\n",
			path_name);
		goto EXIT_LABEL;
	}

    for (i = 1; i < g_dsa_elf_data.ehdr.e_shnum; ++i)
    {
		Elf_Scn *scn;
		GElf_Shdr shdr;
		const char *name;

		scn = elf_getscn(g_dsa_elf_data.elf, i);
		if (scn == NULL || gelf_getshdr(scn, &shdr) == NULL) {
			ds_log("Couldn't get section #%d from"
				" \"%s\": %s\n", i, path_name, elf_errmsg(-1));
			goto EXIT_LABEL;
		}

		name = elf_strptr(g_dsa_elf_data.elf, g_dsa_elf_data.ehdr.e_shstrndx, shdr.sh_name);
		if (name == NULL) {
			ds_log("Couldn't get name of section #%d from"
				" \"%s\": %s\n", i, path_name, elf_errmsg(-1));
			goto EXIT_LABEL;
		}

		if (shdr.sh_type == SHT_SYMTAB)
		{
			dsa_elf_read_symbol_table(&g_dsa_elf_data, path_name,
					  scn, &shdr, name, &g_dsa_elf_data.symtab,
					  &g_dsa_elf_data.symtab_count, &g_dsa_elf_data.strtab);
		    break;

		}
	}

	for (i = 0; i < g_dsa_elf_data.symtab_count; ++i)
	{
	    GElf_Sym sym;
		if (gelf_getsym(g_dsa_elf_data.symtab, i, &sym) == NULL) {
			ds_log(
				"couldn't get symbol #%zd from %s: %s\n",
				i, path_name, elf_errmsg(-1));
			continue;
		}

		if (sym.st_value == 0 || sym.st_shndx == STN_UNDEF
		    /* Also ignore any special values besides direct
		     * section references.  */
		    || sym.st_shndx >= g_dsa_elf_data.ehdr.e_shnum)
			continue;

        INT32 st_info = GELF_ST_TYPE(sym.st_info);

        if ((st_info != STT_FUNC) && (st_info != STT_OBJECT))
        {
            continue;
        }
        
	    const char *orig_name = g_dsa_elf_data.strtab + sym.st_name;
		const char *version = strchr(orig_name, '@');
		size_t len = version != NULL ? (assert(version > orig_name),
						(size_t)(version - orig_name))
			: strlen(orig_name);
		char name[len + 1];
		memcpy(name, orig_name, len);
		name[len] = 0;

        /*ds_log("%s, %x\n", name, sym.st_value);*/
		ds_symbol_add(name, (CHAR *)sym.st_value, 0);
	}

	elf_end(g_dsa_elf_data.elf);
	close(g_dsa_elf_data.fd);
	
    return  TRUE;
EXIT_LABEL:
    elf_end(g_dsa_elf_data.elf);
	close(g_dsa_elf_data.fd);
    return  FALSE;
}

INT32  dsa_elf_init(VOID)
{
    INT32  ret = FALSE;
    CHAR  proc_path[DS_PROC_NAME_LEN] = {0};

    memset(&g_dsa_elf_data, 0, sizeof(DSA_ELF_DATA));
    
    snprintf(proc_path, DS_PROC_NAME_LEN, "/proc/%d/exe", getpid());
    ret = dsa_elf_read_symbols(proc_path);
    
    return  ret;
}
