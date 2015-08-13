
#include "rtl8195a.h"
#include "rt_TypeDef.h"
#include "rt_system.h"
#include "cmsis_os.h"
#include "ticker_api.h"
#include "us_ticker_api.h"
#include "section_config.h"

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

WIFI_RAM_DATA_SECTION
static uint8_t uxTskCriticalNesting = 0;

WIFI_RAM_TEXT_SECTION
void save_and_cli(void)
{
    cli();
	if (uxTskCriticalNesting== 0)
		rt_tsk_lock();
	uxTskCriticalNesting++;
}

WIFI_RAM_TEXT_SECTION
void restore_flags(void)
{
	// assert(uxTskCrticialNesting>0);
	uxTskCriticalNesting--;
	if (uxTskCriticalNesting== 0)
 		rt_tsk_unlock();
	sti();
}

WIFI_RAM_DATA_SECTION
static uint8_t uxCriticalNesting = 0;

WIFI_RAM_TEXT_SECTION
void cli(void)
{
	if (uxCriticalNesting== 0)
		__disable_irq();
	uxCriticalNesting++;
}

WIFI_RAM_TEXT_SECTION
void sti(void)
{
	// assert(uxCrticialNesting>0);
	uxCriticalNesting--;
	if (uxCriticalNesting==0)
		__enable_irq();
}

//
// Time 
//
#define one_us_cycles    ((1000UL-5UL)/6)
#define delay_times (one_us_cycles/6)  

WIFI_RAM_TEXT_SECTION
void rtw_udelay_os(int us)
{
	uint32_t i, j;
	uint32_t us1, us2;


	if ( us > 1000 ) {	
		us1 = us / 1000;
		osDelay(us1);
		us = us % 1000;
	}
	
	// http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0337h/CHDDIGAC.html
	//
	// 1000eb9e:	bf00      	nop		    // 1 cycle
    // 1000eba0:	bf00      	nop            // 1 cycle
    // 1000eba2:	3301      	adds	r3, #1  // 1 cycle
    // 1000eba4:	4293      	cmp	r3, r2   // 1 cycle
    // 1000eba6:	d1fa      	bne.n	1000eb9e <rtw_udelay_os+0xe> // 1 cycle
	//
	
	__disable_irq();
	for (i=0; i<us; i++) {
		for (j=0; j<delay_times; j++) {
			asm volatile (
				"nop" "\n\t"
				"nop"); //just waiting 2 cycle
		}
	}	
	__enable_irq();
}

WIFI_RAM_TEXT_SECTION
void rt_os_mdelay(int ms)
{
	osDelay(ms);
}

//
// memory management
//

WIFI_RAM_TEXT_SECTION
void* rtw_malloc(size_t size) 
{
	return mem_malloc(size);
}

WIFI_RAM_TEXT_SECTION
void rtw_free(void* ptr)
{
	return mem_free(ptr);
}


WIFI_RAM_TEXT_SECTION
int rtw_memcmp(void *dst, void *src, u32 sz)
{
	if ( memcmp(dst, src, sz) == 0 ) return _TRUE;

	return _FALSE;
}

WIFI_RAM_TEXT_SECTION
void rtw_memcpy(void* dst, void* src, u32 sz)
{
    memcpy(dst, src, sz);
}

WIFI_RAM_TEXT_SECTION
void rtw_memset(void *pbuf, int c, u32 sz)
{
	memset(pbuf, c, sz);
}


//osMutexDef(MutexSpinLock);
WIFI_RAM_TEXT_SECTION
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

WIFI_RAM_TEXT_SECTION
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

WIFI_RAM_TEXT_SECTION
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

WIFI_RAM_TEXT_SECTION
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

WIFI_RAM_TEXT_SECTION
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

WIFI_RAM_TEXT_SECTION
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

WIFI_RAM_TEXT_SECTION
int rtw_down_sema(_sema *sema)
{
	return rtw_down_timeout_sema(sema,osWaitForever);
}

//
// random
//
WIFI_RAM_TEXT_SECTION
int rtw_random(void)
{
	u32 res = rtw_get_current_time();
	static unsigned long seed = 0xDEADB00B;
	seed = ((seed & 0x007F00FF) << 7) ^
	    ((seed & 0x0F80FF00) >> 8) ^ // be sure to stir those low bits
	    (res << 13) ^ (res >> 9);    // using the clock too!
	return (int)seed;
}

WIFI_RAM_TEXT_SECTION
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
WIFI_RAM_TEXT_SECTION
u32 rtw_get_current_time(void)
{
	return rt_time_get();
}

WIFI_RAM_TEXT_SECTION
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

// timer


WIFI_RAM_DATA_SECTION
static TIMER_FUN timer_func[MAX_TIMER_ID]={NULL};

WIFI_RAM_DATA_SECTION
static void* timer_func_cntx[MAX_TIMER_ID]={NULL};

WIFI_RAM_DATA_SECTION
static ticker_event_t timer_func_event[MAX_TIMER_ID];

WIFI_RAM_TEXT_SECTION
static void rtw_timer_irq(uint32_t id) 
{
    TIMER_FUN timer_f;

	if ( id <0 || id>=MAX_TIMER_ID ) return;

	timer_f = timer_func[id];
	if ( timer_f == NULL ) return;

	timer_f(timer_func_cntx[id]);
}

WIFI_RAM_TEXT_SECTION
void rtw_init_timer(uint8_t *ptimer_id, TIMER_FUN pfunc,void* cntx, char* name)
{
	const ticker_data_t *_ticker_data;
	int i;


	DiagPrintf("%s %s : ", __FUNCTION__, name);

	for (i=0; i<MAX_TIMER_ID; i++) 
		if (timer_func[i] == NULL) break;

	if ( i >= MAX_TIMER_ID ) {
		DiagPrintf("Can not init timer, already MAX \r\n");
		return;
	}

	*ptimer_id = i;
	timer_func[i] = pfunc;
	timer_func_cntx[i] = cntx;
	
	_ticker_data= get_us_ticker_data();
	ticker_set_handler(_ticker_data, &rtw_timer_irq);
	
	DiagPrintf(" %d \r\n", i);
}


WIFI_RAM_TEXT_SECTION
void rtw_set_timer(uint8_t timer_id, u32 delay_time)
{
	const ticker_data_t *_ticker_data;

	if ( timer_id <0 || timer_id>=MAX_TIMER_ID ) return;
	
	if (timer_func[timer_id] == NULL ) return;
	_ticker_data= get_us_ticker_data();
	ticker_insert_event(_ticker_data, &timer_func_event[timer_id], delay_time*1000+ticker_read(_ticker_data), timer_id);
}

WIFI_RAM_TEXT_SECTION
void rtw_cancel_timer(uint8_t timer_id)
{
	const ticker_data_t *_ticker_data;

	if ( timer_id <0 || timer_id>=MAX_TIMER_ID ) return;
	
	if (timer_func[timer_id] == NULL ) return;
	_ticker_data= get_us_ticker_data();
	ticker_remove_event(_ticker_data, &timer_func_event[timer_id]);
}

WIFI_RAM_TEXT_SECTION
void rtw_del_timer(uint8_t timer_id)
{
	if ( timer_id <0 || timer_id>=MAX_TIMER_ID ) return;

	rtw_cancel_timer(timer_id);	
	timer_func[timer_id] = NULL;
	timer_func_cntx[timer_id] = NULL;
}

