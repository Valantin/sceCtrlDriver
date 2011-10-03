#include <memset.h>

#include <pspctrl.h>
#include <pspctrl_kernel.h>
#include <pspsysevent.h>
#include <pspsystimer.h>
#include <pspsyscon.h>

PSP_MODULE_INFO("sceController_Service", 1007, 1, 10);

#define SLL_K1(k1) asm volatile("sll $k1, $k1, %0;" :: "r" (b))
#define SET_K1(k1) asm volatile("move $k1, %0;" :: "r" (k1))
#define GET_K1(k1) asm volatile("move %0, $k1;" : "=r" (k1))

typedef struct ButtonCallback {
    unsigned int mask;
    void (*cb)(int, int, void*);
    void *gp;
    void *arg;
}

typedef struct sceSysconPacket
{
        u8      unk00[4];               // +0x00 ?(0x00,0x00,0x00,0x00)
        u8      unk04[2];               // +0x04 ?(arg2)
        u8      status;                 // +0x06
        u8      unk07;                  // +0x07 ?(0x00)
        u8      unk08[4];               // +0x08 ?(0xff,0xff,0xff,0xff)
        // transmit data
        u8      tx_cmd;                 // +0x0C command code
        u8      tx_len;                 // +0x0D number of transmit bytes
        u8      tx_data[14];    // +0x0E transmit parameters
        // receive data
        u8      rx_sts;                 // +0x1C generic status
        u8      rx_len;                 // +0x1D receive length
        u8      rx_response;    // +0x1E response code(tx_cmd or status code)
        u8      rx_data[9];             // +0x1F receive parameters
        // ?
        u32     unk28;                  // +0x28
        // user callback (when finish an access?)
        void (*callback)(struct sceSysconPacket *,u32); // +0x2c
        u32     callback_r28;   // +0x30
        u32     callback_arg2;  // +0x34 arg2 of callback (arg4 of sceSycconCmdExec)

        u8      unk38[0x0d];    // +0x38
        u8      old_sts;                // +0x45 old     rx_sts
        u8      cur_sts;                // +0x46 current rx_sts
        u8      unk47[0x21];    // +0x47
        // data size == 0x60 ??
} sceSysconPacket;


PspSysEventHandler sysevent;//0x00002880
typedef struct SceCtrlOption {
    SceSysTimerId timer;        //0x000028C0
    SceUID evid;                //0x000028C4
    int cycle;                  //0x000028C8
    char sampling[2];           //0x000028CC
    char g0x000028CE;           //0x000028CE
    char g0x000028CF;           //0x000028CF
    int g0x000028D0;//run ?     //0x000028D0
                            //0x000028D4
    char polling;               //0x000028D5
    short g0x000028D6;          //0x000028D6
    int g0x000028D8;            //0x000028D8
    sceSysconPacket packet;     //0x000028DC
                                //0x0000293C
                            //0x000029E0
    int rapidfire[16];          //0x000029EC
                            //0x00002A02
    char g0x00002B04;           //0x00002B04
    char g0x00002B05;           //0x00002B05
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
    polling = 1;
    g0x000029B8 = 0x00002BE0;
    g0x00002B04 = -128;
    g0x00002B05 = -128;
    g0x000029E0 = 0x00002FE0;
    g0x000028D8 = -1;
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

void sub_00000968(int arg1, char arg2, char arg3) {
    unsigned int timelow = sceKernelGetSystemTimeLow();
    
}

SceUInt sub_00001E0C (void *common) { //(alarm handler)
    int intr = sceKernelCpuSuspendIntr();
    int var1;
    if(option->g0x000028D8 != 0) var1 = option->g0x00002AEC;
    else asm("ins $a0, $zr, 0, 17" : "=r" var1);
    sub_00000968(ptr, option->g0x00002B04, option->g0x00002B05);
    sceKernelSetEventFlag(option->evid, 1);
    sceKernelCpuResumeIntr(intr);
    return 0;
}

int sub_00000528 () { //not complete (handler)
    int intr = sceKernelCpuSuspendIntr();
    if(option->cycle != 0) {
        if(option->g0x000028D0 == 0 && option->polling != 0) {
            option->g0x000028D0 = 1;
            option->packet->tx_cmd = 7;
            if(option->sampling[0] != 0 && option->sampling[1] != 0)
                option->packet->tx_cmd = 8;
            option->packet->tx_len = 2;
            int ret = sceSysconCmdExecAsync(option->packet, 1, 0x610, 0);
            if(ret < 0) option->g0x000028D0 = 0;
        } else {
            sceKernelSetAlarm(0x2BC, sub_00001E0C, NULL);
        }
    }
    if(option->g0x000028D4 != 0) {
        option->g0x000028D4 = 0;
        sceKernelPowerTick(0);
    }
    sceKernelCpuResumeIntr(intr);
    return -1;
}


int sceCtrlResume() {//not complete
    int var3 = sceSyscon_driver_4717A520();
    if(var3 == 0)
        asm("ins %0, $zr, 29, 1" : "=r" option->g0x00002AF8);
    else if(var3 == 1)
        option->g0x00002AF8 |= 0x20000000;
    option->g0x000028CF = -1;
    if(option->cycle != 0) {
        sceSTimerStartCount(option->timer);
        sceSTimerSetHandler(option->timer, option->cycle, sub_00000528, 0);
    } else {
        sceKernelReleaseSubIntrHandlerFunction(/*PSP_DISPLAY_SUBINT, PSP_THREAD0_INT*/0x1E, 0x13);
	}
    return 0;
}

int sceCtrlSetSamplingMode(int mode) {
    int ret = 0x80000107;
    if(mode < 2) {
        int intr = sceKernelCpuSuspendIntr();
        int k1;
        GET_K1(k1);
        int i = k1 >> 20 & 1;
        ret = sampling[i];
        sampling[i] = mode;
        sceKernelCpuResumeIntr(intr);
    }
    return ret;
}

int sceCtrlGetSamplingMode(int *pmode) {
    int k1;
    GET_K1(k1);
    SLL_K1(11);
    i = (k1 >> 31 < 1) ? 1 : 0;
    if(k1 & pmode >= 0) {
        pmode = sampling[i];
    }
    SET_K1(k1);
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
    int k1;
    GET_K1(k1);
    SLL_K1(11);
    if(k1 & pcycle >= 0)
        pcycle = option->cycle;
    SET_K1(k1);
    return 0;
}

int sub_00001EA4 (SceCtrlData *pad_data, int count, int arg3, int mode) { //not complete
    if(count > 64) return 0x80000104;
    if(arg3 >= 3) return 0x800001FE;
    if(arg3 == 2) return 0x80000004;//GENERIC ERROR
    int k1;
    GET_K1(k1);
    SLL_K1(11);
    if(k1 & pad_data < 0) return 0x80000023;
    if(k1 < 0) {
        if(
    }
    SET_K1(k1);
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