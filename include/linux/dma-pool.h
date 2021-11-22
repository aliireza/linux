#ifndef _DMA_POOL_
#define _DMA_POOL_

#include <linux/wait.h>
#include <linux/compiler.h>
#include <linux/dma-direction.h>

#define CEIL(x, y)        (((x) + ((y)-1)) / (y))

typedef struct dmapool_s {
      spinlock_t lock;
      u64 page_nr;  
      u64 curr_page_nr;
      struct page **page_array;
      gfp_t gfp_mask;
      int nid;

      int hugepage_nr;
      int curr_hugepage_nr;
      struct page **hugepage_array;
      int hugepage_order;

      struct device *dev;
      enum dma_data_direction dma_dir;
} dmapool_t;

dmapool_t *dmapool_create(u64 page_nr, int hugepage_order, gfp_t gfp_mask, int nid, struct device *dev, enum dma_data_direction dir);
void dmapool_destroy(dmapool_t *pool);

struct page *dmapool_alloc_page(dmapool_t *pool);
void dmapool_free_page(struct page *page, dmapool_t *pool);


#endif /*_DMA_POOL_*/ 
