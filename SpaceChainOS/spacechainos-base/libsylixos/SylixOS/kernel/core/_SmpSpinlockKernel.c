/*********************************************************************************************************
**
**                                    �й�������Դ��֯
**
**                                   Ƕ��ʽʵʱ����ϵͳ
**
**                                       SylixOS(TM)
**
**                               Copyright  All Rights Reserved
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**
** ��   ��   ��: _SmpSpinlockKernel.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2015 �� 11 �� 13 ��
**
** ��        ��: �� CPU ϵͳ�ں�������.
** 
** ע        ��: �ں��������������ڲ�����̽�����н��е�, �ڼ���Ҫ���ж�.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "../SylixOS/kernel/include/k_kernel.h"
/*********************************************************************************************************
  �ü�֧��
*********************************************************************************************************/
#if LW_CFG_SMP_EN > 0
/*********************************************************************************************************
  ��������������ֵ
*********************************************************************************************************/
#define LW_SPIN_OK      1
#define LW_SPIN_ERROR   0
/*********************************************************************************************************
  �ں�������
*********************************************************************************************************/
#define LW_KERN_SL      (_K_klKernel.KERN_slLock)
/*********************************************************************************************************
** ��������: _SmpSpinLockIgnIrq
** ��������: �ں���������������, �����ж����� (�������жϹرյ�״̬�±�����)
** �䡡��  : NONE
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _SmpKernelLockIgnIrq (VOID)
{
    PLW_CLASS_CPU   pcpuCur;
    INT             iRet;
    
    pcpuCur = LW_CPU_GET_CUR();
    
    for (;;) {
        iRet = __ARCH_SPIN_TRYLOCK(&LW_KERN_SL);
        if (iRet != LW_SPIN_OK) {
            _SmpTryProcIpi(pcpuCur);                                    /*  ����ִ�� IPI                */
            LW_SPINLOCK_DELAY();
            
        } else {
            if (!pcpuCur->CPU_ulInterNesting) {
                __THREAD_LOCK_INC(pcpuCur->CPU_ptcbTCBCur);             /*  ���������ڵ�ǰ CPU          */
            }
            KN_SMP_MB();
            break;
        }
    }
}
/*********************************************************************************************************
** ��������: _SmpKernelUnlockIgnIrq
** ��������: �ں���������������, �����ж����� (�������жϹرյ�״̬�±�����)
** �䡡��  : NONE
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _SmpKernelUnlockIgnIrq (VOID)
{
    PLW_CLASS_CPU   pcpuCur;
    INT             iRet;
    
    KN_SMP_MB();
    iRet = __ARCH_SPIN_UNLOCK(&LW_KERN_SL);
    _BugFormat((iRet != LW_SPIN_OK), LW_TRUE, "unlock error %p!\r\n", &LW_KERN_SL);
    
    pcpuCur = LW_CPU_GET_CUR();
    if (!pcpuCur->CPU_ulInterNesting) {
        __THREAD_LOCK_DEC(pcpuCur->CPU_ptcbTCBCur);                     /*  �����������                */
    }
}
/*********************************************************************************************************
** ��������: _SmpKernelLockQuick
** ��������: �ں���������������, ��ͬ�����ж�
** �䡡��  : piregInterLevel   �ж�������Ϣ
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _SmpKernelLockQuick (INTREG  *piregInterLevel)
{
    PLW_CLASS_CPU   pcpuCur;
    INT             iRet;
    
    for (;;) {
        *piregInterLevel = KN_INT_DISABLE();
        
        iRet = __ARCH_SPIN_TRYLOCK(&LW_KERN_SL);
        if (iRet != LW_SPIN_OK) {
            _SmpTryProcIpi(LW_CPU_GET_CUR());                           /*  ����ִ�� IPI                */
            KN_INT_ENABLE(*piregInterLevel);
            LW_SPINLOCK_DELAY();
            
        } else {
            pcpuCur = LW_CPU_GET_CUR();
            if (!pcpuCur->CPU_ulInterNesting) {
                __THREAD_LOCK_INC(pcpuCur->CPU_ptcbTCBCur);             /*  ���������ڵ�ǰ CPU          */
            }
            KN_SMP_MB();
            break;
        }
    }
}
/*********************************************************************************************************
** ��������: _SmpKernelUnlockQuick
** ��������: �ں���������������, ��ͬ�����ж�, �����г��Ե���
** �䡡��  : iregInterLevel    �ж�������Ϣ
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _SmpKernelUnlockQuick (INTREG  iregInterLevel)
{
    PLW_CLASS_CPU   pcpuCur;
    INT             iRet;
    
    KN_SMP_MB();
    iRet = __ARCH_SPIN_UNLOCK(&LW_KERN_SL);
    _BugFormat((iRet != LW_SPIN_OK), LW_TRUE, "unlock error %p!\r\n", &LW_KERN_SL);
    
    pcpuCur = LW_CPU_GET_CUR();
    if (!pcpuCur->CPU_ulInterNesting) {
        __THREAD_LOCK_DEC(pcpuCur->CPU_ptcbTCBCur);                     /*  �����������                */
    }
    
    KN_INT_ENABLE(iregInterLevel);
}
/*********************************************************************************************************
** ��������: _SmpKernelUnlockSched
** ��������: �ں� SMP �������л���ɺ�ר���ͷź��� (���ж�״̬�±�����)
** �䡡��  : ptcbOwner     ���ĳ�����
** �䡡��  : NONE
** ȫ�ֱ���: 
** ����ģ��: 
*********************************************************************************************************/
VOID  _SmpKernelUnlockSched (PLW_CLASS_TCB  ptcbOwner)
{
    PLW_CLASS_CPU   pcpuCur;
    INT             iRet;
    
    KN_SMP_MB();
    iRet = __ARCH_SPIN_UNLOCK(&LW_KERN_SL);
    _BugFormat((iRet != LW_SPIN_OK), LW_TRUE, "unlock error %p!\r\n", &LW_KERN_SL);
    
    pcpuCur = LW_CPU_GET_CUR();
    if (!pcpuCur->CPU_ulInterNesting) {
        __THREAD_LOCK_DEC(ptcbOwner);                                   /*  �����������                */
    }
}

#endif                                                                  /*  LW_CFG_SMP_EN               */
/*********************************************************************************************************
  END
*********************************************************************************************************/