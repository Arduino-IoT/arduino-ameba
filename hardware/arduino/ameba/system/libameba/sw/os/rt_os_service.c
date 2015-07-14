
#include "rtl8195a.h"
#include "rt_TypeDef.h"
#include "rt_system.h"
#include "cmsis_os.h"


#include "rt_os_service.h"

static unsigned int __div64_32(u64 *n, unsigned int base)
{
	u64 rem = *n;
	u64 b = base;
	u64 res, d = 1;
	unsigned int high = rem >> 32;

	/* Reduce the thing a bit first */
	res = 0;
	if (high >= base) {
		high /= base;
		res = (u64) high << 32;
		rem -= (u64) (high * base) << 32;
	}

	while ((u64)b > 0 && b < rem) {
		b = b+b;
		d = d+d;
	}

	do {
		if (rem >= b) {
			rem -= b;
			res += d;
		}
		b >>= 1;
		d >>= 1;
	} while (d);

	*n = res;
	return rem;
}


//
// public functions
//

void save_and_cli(void)
{
	rt_tsk_lock();
}

void restore_flags(void)
{
	rt_tsk_unlock();
}

void cli(void)
{
	__disable_irq();
}

void sti(void)
{
	__enable_irq();
}

//
// Time 
//
void rtw_udelay_os(int us)
{
	HalDelayUs(us);
}

void rt_os_mdelay(int ms)
{
	osDelay(ms);
}

//
// memory management
//


int rtw_memcmp(void *dst, void *src, u32 sz)
{
	if ( memcmp(dst, src, sz) == 0 ) return _TRUE;

	return _FALSE;
}

void rtw_memcpy(void* dst, void* src, u32 sz)
{
    memcpy(dst, src, sz);
}

void rtw_memset(void *pbuf, int c, u32 sz)
{
	memset(pbuf, c, sz);
}


//osMutexDef(MutexSpinLock);

void rtw_spinlock_init(_lock *plock)
{


//
// MUTEX : mbed return parameter error. fix it some other time
// 
#if 0
	osMutexId mutex_id;

	mutex_id = osMutexCreate( osMutex(MutexSpinLock) );
	if ( mutex_id == NULL ) {
		DiagPrintf("%s failed, mutex_id == NULL \r\n", __FUNCTION__ );
	}
	*plock = (_lock)mutex_id;
#endif
}


void rtw_spinlock_free(_lock *plock)
{
#if 0
	osMutexId mutex_id;

	if ( plock == NULL ) return;
	mutex_id = *plock;

	if ( mutex_id != NULL )
		osMutexRelease(mutex_id);	
#endif
}


int rtw_create_thread(osThreadDef_t* pthread_def, void (*task)(), void *argument,
        osPriority priority, uint32_t stack_size)
{
	int tid;
	
    pthread_def->pthread = task;
    pthread_def->tpriority = priority;
    pthread_def->stacksize = stack_size;
    pthread_def->stack_pointer = (unsigned char*)mem_malloc(stack_size);
    if (pthread_def->stack_pointer == NULL) {
        DiagPrintf("Thread: Error allocating the stack memory\n");
		return 0;
    }

	tid = osThreadCreate(pthread_def, argument);
	return tid;
	
}

//
// Semaphore
// 

struct _sema_T {
	osSemaphoreId _osSemaphoreId;
	osSemaphoreDef_t _osSemaphoreDef;
	u32 _semaphore_data[2];
};

void rtw_init_sema(_sema *sema, int count)
{
	struct _sema_T *pSema;
	
	if (sema == NULL ) {
		DiagPrintf("%s : sema == NULL", __FUNCTION__);
		return;
	}
	
	pSema = (struct _sema_T*)malloc(sizeof(struct _sema_T));
	if ( pSema == NULL ) {
		DiagPrintf("%s : psema == NULL", __FUNCTION__);
		return;
	}
	
	memset(pSema->_semaphore_data, 0, sizeof(pSema->_semaphore_data));
	pSema->_osSemaphoreDef.semaphore = pSema->_semaphore_data;
	pSema->_osSemaphoreId = osSemaphoreCreate(&pSema->_osSemaphoreDef,count);
	DiagPrintf("%s : id = 0x%x \r\n", __FUNCTION__, pSema->_osSemaphoreId);

	*sema = pSema;
}

int rtw_up_sema(_sema *sema)
{
	struct _sema_T *pSema;
	osSemaphoreId id;	
	
	if (sema == NULL ) {
		DiagPrintf("%s : sema == NULL", __FUNCTION__);
		return;
	}
	
	pSema = *sema;
	if ( pSema == NULL ) {
		DiagPrintf("%s : psema == NULL", __FUNCTION__);
		return;
	}

	id = pSema->_osSemaphoreId;
	//DiagPrintf("%s : id = 0x%x \r\n", __FUNCTION__, pSema->_osSemaphoreId);
	return osSemaphoreRelease(id);
	
}

int	rtw_down_timeout_sema(_sema *sema, u32 timeout_ms)
{
	struct _sema_T *pSema;
	osSemaphoreId id;	
	
	if (sema == NULL ) {
		DiagPrintf("%s : sema == NULL", __FUNCTION__);
		return;
	}
	
	pSema = *sema;
	if ( pSema == NULL ) {
		DiagPrintf("%s : psema == NULL", __FUNCTION__);
		return;
	}

	id = pSema->_osSemaphoreId;
	//DiagPrintf("%s : id = 0x%x \r\n", __FUNCTION__, pSema->_osSemaphoreId);
	return osSemaphoreWait(id, timeout_ms);
}

int rtw_down_sema(_sema *sema)
{
	return rtw_down_timeout_sema(sema,osWaitForever);
}

//
// random
//
int rtw_random(void)
{
	u32 res = rtw_get_current_time();
	static unsigned long seed = 0xDEADB00B;
	seed = ((seed & 0x007F00FF) << 7) ^
	    ((seed & 0x0F80FF00) >> 8) ^ // be sure to stir those low bits
	    (res << 13) ^ (res >> 9);    // using the clock too!
	return (int)seed;
}

void rtw_get_random_bytes(u8 *buf, int len)
{
	unsigned int ranbuf;
	unsigned int *lp;
	int i, count;
	count = len / sizeof(unsigned int);
	lp = (unsigned int *) buf;

	for(i = 0; i < count; i ++) {
		lp[i] = rtw_random();  
		len -= sizeof(unsigned int);
	}

	if(len > 0) {
		ranbuf = rtw_random();
		memcpy(&lp[i], &ranbuf, len);
	}
}



//
// Time management
//

u32 rtw_get_current_time(void)
{
	return rt_time_get();
}


u64 rtw_modular64(u64 n, u64 base)
{
	unsigned int __base = (base);
	unsigned int __rem;

	if (((n) >> 32) == 0) {
		__rem = (unsigned int)(n) % __base;
		(n) = (unsigned int)(n) / __base;
	}
	else
		__rem = __div64_32(&(n), __base);
	
	return __rem;
}

