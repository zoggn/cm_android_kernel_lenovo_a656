
#include "mtk_sysfreq.h"
#include "mach/mt_gpufreq.h"
#include "mach/mt_clkmgr.h"
#include <mach/sync_write.h>

static bool mt_keep_freq_non_od_set = false;

#define MTK_GPU_DVFS 0
#define PROTON_KEEP_FREQ 0

/* SGX544 GPU overclock overrides */
#if 0
	#define MTK_GPU_OC_476MHz
#endif
#if 0
	#define MTK_GPU_OC_403MHz
#endif
#if 0
	#define MTK_FORCE_T /* 357MHz */
#endif
#if 0
	#define MTK_FORCE_312MHz
#endif
#if 1
	#define MTK_FORCE_6589 /* 286MHz - force 6589M to 6589 GPU clock */
#endif

#if MTK_GPU_DVFS
static struct mt_gpufreq_info freqs_special_vrf18_2[] = {
    {GPU_DVFS_F3, 40, 100, GPU_POWER_VRF18_1_05V, 100},
    {GPU_DVFS_F5, 0,  40, GPU_POWER_VRF18_1_05V,  80},
};
static struct mt_gpufreq_info freqs_special2_vrf18_2[] = {
    {GPU_DVFS_F2, 60, 100, GPU_POWER_VRF18_1_05V, 100},
    {GPU_DVFS_F3, 30,  60, GPU_POWER_VRF18_1_05V,  90},
    {GPU_DVFS_F5, 0,  30, GPU_POWER_VRF18_1_05V,  75},
};
static struct mt_gpufreq_info freqs_special3_vrf18_2[] = {
    {GPU_DVFS_F1, 60, 100, GPU_POWER_VRF18_1_05V, 100},
    {GPU_DVFS_F3, 30,  60, GPU_POWER_VRF18_1_05V,  90},
    {GPU_DVFS_F5, 0,  30, GPU_POWER_VRF18_1_05V,  75},
};
#endif

void MtkInitSetFreqTbl(unsigned int tbltype)
{
    switch (tbltype)
    {
#if MTK_GPU_DVFS
    case TBLTYPE1:
//        printk("[GPU DVFS] register vrf18_2 special table ...\n");
        mt_gpufreq_register(freqs_special_vrf18_2, 2);
        break;
    case TBLTYPE2:
//        printk("[GPU DVFS] register vrf18_2 special2 table ...\n");
        mt_gpufreq_register(freqs_special2_vrf18_2, 3);
        break;
    case TBLTYPE3:
//        printk("[GPU DVFS] register vrf18_2 special3 table ...\n");
        mt_gpufreq_register(freqs_special3_vrf18_2, 3);
        break;
#endif
    default:
    case TBLTYPE0:
        mt_gpufreq_non_register();
        break;
    }
}


PVRSRV_ERROR MTKSetFreqInfo(unsigned int freq, unsigned int tbltype)
{
	tbltype = TBLTYPE0;
	
    printk(" freq= %d", freq);
#if defined(MTK_GPU_OC_476MHz)
    freq = GPU_DVFS_F1;
    tbltype = TBLTYPE3;
#endif      
#if defined(MTK_GPU_OC_403MHz)
    freq = GPU_DVFS_F2;
    tbltype = TBLTYPE2;
#endif    
#if defined(MTK_FORCE_T)
    freq = GPU_DVFS_F3;
    tbltype = TBLTYPE1;
#endif
#if defined(MTK_FORCE_312MHz)
    freq = GPU_DVFS_F4;
    tbltype = TBLTYPE0;
#endif
#if defined(MTK_FORCE_6589)
    freq = GPU_DVFS_F5;
    tbltype = TBLTYPE0;
#endif
#if defined(MTK_FORCE_M)
    freq = GPU_DVFS_F7;
    tbltype = TBLTYPE0;
#endif


//#if defined(MTK_FREQ_OD_INIT)
    if (freq > GPU_DVFS_F5)
    {
        mt_gpufreq_set_initial(freq, GPU_POWER_VRF18_1_05V);
        mt65xx_reg_sync_writel((readl(CLK_CFG_8)&0xffcffff)|0x30000, CLK_CFG_8);
        #if defined(MTK_GPU_OC_476MHz)
			mt_gpufreq_keep_frequency_non_OD_init(GPU_MMPLL_D3, GPU_POWER_VRF18_1_05V);
		#endif
		#if defined(MTK_GPU_OC_403MHz)
			mt_gpufreq_keep_frequency_non_OD_init(GPU_SYSPLL_D2, GPU_POWER_VRF18_1_05V);
		#endif
		#if defined(MTK_FORCE_T)
			mt_gpufreq_keep_frequency_non_OD_init(GPU_MMPLL_D4, GPU_POWER_VRF18_1_05V);
		#endif
    }
    else
//#endif
    {
        mt_gpufreq_set_initial(freq, GPU_POWER_VRF18_1_05V);
        mt_gpufreq_keep_frequency_non_OD_init(GPU_KEEP_FREQ_NON_OD_BYPASS, GPU_KEEP_VOLT_NON_OD_BYPASS);
    }
//        mt_gpufreq_keep_frequency_non_OD_init(GPU_KEEP_FREQ_NON_OD_BYPASS, GPU_KEEP_VOLT_NON_OD_BYPASS);

    MtkInitSetFreqTbl(tbltype);

    return PVRSRV_OK;
}

void MtkSetKeepFreq(void)
{
	#if PROTON_KEEP_FREQ == 0
		if (mt_gpufreq_keep_frequency_non_OD_get())
		{
			if (mt_keep_freq_non_od_set==false)
			{
	#endif
				mt_gpufreq_keep_frequency_non_OD_set(1);
				mt_gpufreq_keep_frequency_non_OD(1);
				mt_keep_freq_non_od_set=true;
	#if PROTON_KEEP_FREQ == 0
			}
	
		}
		else
		{
			if (mt_keep_freq_non_od_set==true)
			{
				mt_gpufreq_keep_frequency_non_OD_set(0);
				mt_keep_freq_non_od_set=false;
			}
		}
	#endif
}

