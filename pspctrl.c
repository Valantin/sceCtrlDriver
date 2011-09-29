#include <memset.h>

#include <pspctrl.h>
#include <pspctrl_kernel.h>
#include <pspsysevent.h>
#include <pspsystimer.h>
#include <pspsyscon.h>

PSP_MODULE_INFO("sceController_Service", 1007, 1, 10);

typedef struct ButtonCallback {
    unsigned int mask;
    void (*cb)(int, int, void*);
    void *gp;
    void *arg;
}

PspSysEventHandler sysevent;//0x00002880
typedef struct SceCtrlOption {
    SceSysTimerId timer;        //0x000028C0
    SceUID evid;                //0x000028C4
    int cycle;                  //0x000028C8
    char sampling[2];           //0x000028CC
    char g0x000028CE;           //0x000028CE
    int g0x000028D0;            //0x000028D0
                            //0x000028D4
    char polling;               //0x000028D5
    short g0x000028D6;          //0x000028D6
    int g0x000028D8;            //0x000028D8
                            //0x000029E0
    int rapidfire[16];          //0x000029EC
                            //0x00002A02
    char g0x00002B07;           //0x00002B07
    int g0x00002B58;            //0x00002B58
    int g0x00002B5C;            //0x00002B5C
    int g0x00002B64;            //0x00002B64
    int g0x00002B68;            //0x00002B68
    int g0x00002B6C;            //0x00002B6C
    int g0x00002B70;            //0x00002B70
    int g0x00002B74;            //0x00002B74
    int g0x00002B7C;            //0x00002B7C
    ButtonCallback callback[4]; //0x00002B80 - 0x00002BBF
    int g0x00002BC0;            //0x00002BC0
                            //0x00002BD4
} SceCtrlOption;

SceCtrlOption option;//0x000028C0

int module_start(SceSize args, void* argp) {
    sceCtrlInit();
    return 0;
}

int module_reboot_before(SceSize args, void *argp) {
    sceCtrlEnd();
    return 0;
}

int sceCtrlInit() {//not complete
    memset(&option, 0, 0x314);
    option->evid = sceKernelCreateEventFlag("SceCtrl", 1, 0, NULL);
    option->timer = sceSTimerAlloc();
    if(option->timer != NULL) {
        SysTimerForKernel_CCC7A9E4(option->timer, 0x1, 0x30);
        sceKernelRegisterSysEventHandler(sysevent);
        sceSyscon_driver_62012EAF(0);
        int keyconf = sceKernelInitKeyConfig();
        if(keyconf == 0x110) {
            var = 0x0003FFFF;
        } else if(keyconf == PSP_INIT_KEYCONFIG_VSH) {
            var = 0x0003FFFF;
        } else if(keyconf == PSP_INIT_KEYCONFIG_POPS) {
            var = 0x0003FFFF;
        } else {
            var = 0x0003F3F9;
        }
        g0x00002B60 = 0;
        g0x00002B5C = g0x00002B58;
        g0x00002B58 = var;
        switch(sceKernelGetModel()){
            case 0:
            case 1:
            case 2:
            case 3:
            case 6:
            case 8:
            case 10:
                var26 = 0x01FFF3F9;
                break;
            case 4:
            case 5:
            case 7:
            case 9:
                var26 = 0x39FFF3F9;
                break;
        }
        g0x00002B64 = var26;
        g0x00002B68 = 0x39FFF3F9;
        g0x00002B6C = 0x00F1F3F9;
        g0x00002B70 = 0x390E0000;
        g0x00002B74 = 0;
        g0x00002B78 = 0x81;
        g0x00002B7C = 0x81;
    }
    return 0;
}

int sceCtrlEnd() {
    sceKernelUnregisterSysEventHandler(sysevent);
    sceSyscon_driver_62012EAF(1);
    sceDisplayWaitVblankStart();
    option->timer = -1;
    if(option->timer >= 0) {
        sceSTimerStopCount(option->timer);
        sceSTimerFree(option->timer);
    }
    sceKernelReleaseSubIntrHandlerFunction(/*PSP_DISPLAY_SUBINT, PSP_THREAD0_INT*/0x1E, 0x13);
	sceKernelDeleteEventFlag(option->evid);
    while(option->g0x000028D0)
        sceDisplayWaitVblankStart();
    return 0;
}

int sceCtrlSuspend() {
    if(option->cycle != 0) {
        sceSTimerStopCount(option->timer);
    } else {
        sceKernelDisableSubIntr(PSP_DISPLAY_SUBINT,/*PSP_THREAD0_INT*/ 0x13);
    }
    return 0;
}

int sceCtrlResume() {//not complete
    int var3 = sceSyscon_driver_4717A520();
    return 0;
}

int sceCtrlSetSamplingMode(int mode) {
    int ret = 0x80000107;
    if(mode < 2) {
        int intr = sceKernelCpuSuspendIntr();
        int k1 = pspSdkGetK1();
        int i = k1 >> 20 & 1;
        ret = sampling[i];
        sampling[i] = mode;
        sceKernelCpuResumeIntr(intr);
    }
    return ret;
}

int sceCtrlGetSamplingMode(int *pmode) {
    int k1 = pspSdkSetK1(0);
    i = (k1 >> 31 < 1) ? 1 : 0;
    if(k1 & pmode >= 0) {
        pmode = sampling[i];
    }
    pspSdkSetK1(k1);
    return 0;
}

int sceCtrl_driver_A81235E5(int arg) {
    option->g0x00002B07 = arg;
    return 0;
}

int sceCtrl_driver_7A6436DE(int arg) {
    option->g0x00002BC0 = arg;
    return 0;
}

int sceCtrlSetPollingMode(char arg) {
    option->polling = arg;
    return 0;
}

int sceCtrl_driver_5BE1D4F2() {
    option->g0x000028CE = 1;
    return 0;
}

int sceCtrlSetIdleCancelKey(int arg1, int arg2, int arg3, int arg4) {
    g0x00002B68 = arg1;
    g0x00002B6C = arg2;
    g0x00002B70 = arg3;
    g0x00002B74 = arg4;
    return 0;
}

int sceCtrlSetRapidFire(arg1, arg2, arg3, arg4, arg5, arg6, arg7) {//not complete
    
    return 0;
}

int sceCtrlClearRapidFire(char key) {
    if(key >= 16) return 0x80000102;
    option->rapidfire[key] = 0;
    return 0;
}

int sceCtrl_driver_B7D6332B(short arg1) {//set
    if(arg1 >= 301) return 0x800001FE;
    option->g0x000028D6 = (arg1 ^ 0x1) ? arg1 : 0;
    return 0;
}

int sceCtrl_driver_D2EC6240() {//get
    return option->g0x000028D6;
}

int sceCtrl_driver_A81235E5(char arg1) {
    option->g0x00002B07 = arg1;
    return 0;
}

int sceCtrl_driver_7A6436DE(int arg1) {
    option->g0x00002BC0 = arg1;
    return 0;
}

int sceCtrlRegisterButtonCallback(int no, unsigned int mask, void (*cb)(int, int, void*), void *arg) {
    if(no > 3) return 0x80000102;
    int intr = sceKernelCpuSuspendIntr();
    option->callback[no]->mask = mask;
    option->callback[no]->cb = cb;
    option->callback[no]->gp = $gp;
    option->callback[no]->arg = arg;
    sceKernelCpuResumeIntr(intr);
    return 0;
}

int sceCtrlGetSamplingCycle(int *pcycle) {
    int k1 = pspSdkSetK1(0);
    if(k1 & pcycle >= 0)
        pcycle = option->cycle;
    pspSdkSetK1(k1);
    return 0;
}

int sub_00001EA4 (SceCtrlData *pad_data, int count, int arg3, int mode) { //not complete
    if(count > 64) return 0x80000104;
    if(arg3 >= 3) return 0x800001FE;
    if(arg3 == 2) return 0x80000004;//GENERIC ERROR
    int k1 = pspSdkSetK1(0);
    if(k1 & pad_data < 0) return 0x80000023;
    if(k1 < 0) {
        if(
    }
    pspSdkSetK1(k1);
    return 0;
}

int sceCtrlPeekBufferPositive(SceCtrlData *pad_data, int count) {
    return sub_00001EA4 (pad_data, count, 0, 0);
}

int sceCtrlPeekBufferNegative(SceCtrlData *pad_data, int count) {
    return sub_00001EA4 (pad_data, count, 0, 1);
}

int sceCtrlReadBufferPositive(SceCtrlData *pad_data, int count) {
    return sub_00001EA4 (pad_data, count, 0, 2);
}

int sceCtrlReadBufferNegative(SceCtrlData *pad_data, int count) {
    return sub_00001EA4 (pad_data, count, 0, 3);
}

int sceCtrl_driver_E8121137 (int arg1, SceCtrlData *pad_data, int count) {
    return sub_00001EA4 (pad_data, count, arg1, 4);
}

int sceCtrl_driver_52404C02 (int arg1, SceCtrlData *pad_data, int count) {
    return sub_00001EA4 (pad_data, count, arg1, 5);
}

int sceCtrl_driver_1A5393EC (int arg1, SceCtrlData *pad_data, int count) {
    return sub_00001EA4 (pad_data, count, arg1, 6);
}

int sceCtrl_driver_F8508E92 (int arg1, SceCtrlData *pad_data, int count) {
    return sub_00001EA4 (pad_data, count, arg1, 7);
}