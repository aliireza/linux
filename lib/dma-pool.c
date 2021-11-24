#include <linux/dma-pool.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <asm/bug.h>

/*
 * dmapool_refill  - refill the pool with one hugepage
 * @pool:            the name of the dmapool
 * 
 */
static inline int dmapool_refill(dmapool_t *pool) {
	struct page *pages;
	int i;
	dma_addr_t dma = 0;


	pages = alloc_pages_node(pool->nid, pool->gfp_mask,  pool->hugepage_order);
	if (unlikely(!pages)) {
		return -ENOMEM;
	}
	
	pool->hugepage_array[pool->curr_hugepage_nr++]=pages;
	// printk(KERN_INFO"%d hugepage added\n", pool->curr_hugepage_nr);
        /* dma_map hugepages */
	if(pool->dev) {
		dma = dma_map_page_attrs(pool->dev, pages, 0,
				(PAGE_SIZE << pool->hugepage_order),
				pool->dma_dir, DMA_ATTR_SKIP_CPU_SYNC);
		if (dma_mapping_error(pool->dev, dma)) {
			__free_pages(pages,pool->hugepage_order);
			return -ENOMEM;
		}
        	pages->dma_addr = dma;
	}
        /* Populate 4-KB page array */
	for(i=0;i< (1<<pool->hugepage_order);i++) {
		(pages+i)->dp=pool;
		if(pool->dev)
                	(pages+i)->dma_addr = dma + PAGE_SIZE*i;
		if(!page_ref_count(pages+i))
			page_ref_inc(pages+i);
		pool->page_array[pool->curr_page_nr++]=(pages+i);
	}

	// printk(KERN_INFO"%d hugepage added: pages %lld\n", pool->curr_hugepage_nr, pool->curr_page_nr);

        return 1;
}

/*
 * dmapool_create  - create a dma pool
 * @page_nr:            the number of 4-KB pages
 * @hugepage_order:     the order of hugepage pages to use
 * @gfp_mask:           the allocation bitmask
 * @nid:                the NUMA node id
 * @dev:                the device used for DMA-ing
 * @dir:                the DMA direction
 * 
 * This function creates a pool of 4-KB pages backed by hugepages 
 */
dmapool_t *dmapool_create(u64 page_nr, int hugepage_order, gfp_t gfp_mask, int nid, struct device *dev, enum dma_data_direction dir) {
        dmapool_t *pool;

        pool = kzalloc_node(sizeof(*pool), gfp_mask, nid);
	if (!pool){
		printk(KERN_ERR "dmapool: pool cannot allocated!\n");
		return NULL;
	}
		
        
        if(hugepage_order < 0) {
		printk(KERN_ERR "dmapool: hugepage_order less than zero!\n");
		return NULL;
	}

	spin_lock_init(&pool->lock);
        pool->hugepage_nr = CEIL(page_nr,(1<<hugepage_order));
        pool->page_nr = (1<<hugepage_order)*pool->hugepage_nr;
        pool->hugepage_order = hugepage_order;
        pool->curr_page_nr = 0;
        pool->curr_hugepage_nr = 0;
        pool->dev = dev;
        pool->dma_dir = dir;
        pool->nid = nid;
        pool->gfp_mask = gfp_mask;

	if(pool->dev)
		get_device(pool->dev);
	else
		printk(KERN_WARNING "dmapool: no device\n");

        /* Allocate page arrays */

	pool->page_array = kmalloc_array_node(pool->page_nr, sizeof(struct page*),
					    gfp_mask, nid);
	if(!pool->page_array)
		return NULL;

	pool->hugepage_array = kmalloc_array_node(pool->hugepage_nr, sizeof(struct page*),
					    gfp_mask, nid);
	if(!pool->hugepage_array)
		return NULL;

        /* Allocate Hugepages */

	// printk(KERN_INFO "pool->hugepage_nr %d pool->page_nr %lld\n", pool->hugepage_nr,pool->page_nr);

	while (pool->curr_hugepage_nr < pool->hugepage_nr) {
                if(!dmapool_refill(pool)) {
                        printk(KERN_ERR "dmapool_refill failed!\n");
			return NULL;
                }      
	}
	// printk(KERN_INFO "while loop ended!%lld %d\n",pool->curr_page_nr, (1<<pool->hugepage_order)*pool->curr_hugepage_nr);

        BUG_ON(pool->curr_page_nr != (1<<pool->hugepage_order)*pool->curr_hugepage_nr);

        return pool;
}
EXPORT_SYMBOL(dmapool_create);

/*
 * dmapool_destroy  - destroy a dma pool
 * @pool:       the name of the dmapool
 * 
 * This function destroy a dmapool by freeing the hugepages.
 * Note that the user should ensure that all 4-KB pages are returned to the pool.
 */
void dmapool_destroy(dmapool_t *pool) {
	struct page* tmp;
	
	if (in_serving_softirq())
		spin_lock(&pool->lock);
	else
		spin_lock_bh(&pool->lock);
	if(pool->curr_page_nr == pool->page_nr) {
		while(pool->curr_hugepage_nr) {
			if(pool->dev)
				printk(KERN_INFO "TODO: unmap\n");
			tmp = pool->hugepage_array[--pool->curr_hugepage_nr];
			tmp->dp = NULL;
			put_page(tmp);
			// __free_pages(pool->hugepage_array[--pool->curr_hugepage_nr],pool->hugepage_order);
		}
	} else {
		printk(KERN_ERR "User should return all pages back to the dmapool!\n");
	}
	if (in_serving_softirq())
		spin_unlock(&pool->lock);
	else
		spin_unlock_bh(&pool->lock);

	if(pool->dev)
		put_device(pool->dev);

        printk(KERN_WARNING "dmapool_destroy: to be completed!");
}
EXPORT_SYMBOL(dmapool_destroy);

/*
 * dmapool_alloc_page  - allocate a 4-KB page from the dmapool
 * @pool:       the name of the dmapool
 * 
 */
struct page *dmapool_alloc_page(dmapool_t *pool) {
        struct page *page;

get_page:
	if (in_serving_softirq())
		spin_lock(&pool->lock);
	else
		spin_lock_bh(&pool->lock);
	if (likely(pool->curr_page_nr)) {
		page = pool->page_array[--pool->curr_page_nr];
		if (in_serving_softirq())
			spin_unlock(&pool->lock);
		else
			spin_unlock_bh(&pool->lock);
		/* paired with rmb in mempool_free(), read comment there */
		smp_wmb();
		trace_printk("dmapool_alloc_page\n");
		/*
		 * Update the allocation stack trace as this is more useful
		 * for debugging.
		 */
		return page;
	} else {
                printk(KERN_WARNING "dmapool empty!\n");
		if (in_serving_softirq())
			spin_lock(&pool->lock);
		else
			spin_lock_bh(&pool->lock);
                pool->page_nr += (1<<pool->hugepage_order);
                pool->hugepage_nr++;
                dmapool_refill(pool);
		if (in_serving_softirq())
			spin_unlock(&pool->lock);
		else
			spin_unlock_bh(&pool->lock);
                goto get_page;
        }
}
EXPORT_SYMBOL(dmapool_alloc_page);

/*
 * dmapool_free_page  - return a 4-KB page to the dmapool
 * @pool:       the name of the dmapool
 * 
 */
void dmapool_free_page(struct page *page, dmapool_t *pool) {

	if (unlikely(page == NULL))
		return;
        
        smp_rmb();

	if (likely(pool->curr_page_nr < pool->page_nr)) {
		if (in_serving_softirq())
			spin_lock(&pool->lock);
		else
			spin_lock_bh(&pool->lock);
		if (likely(pool->curr_page_nr < pool->page_nr)) {
			trace_printk("dmapool_free_page!\n");
		        pool->page_array[pool->curr_page_nr++]=page;
			if (in_serving_softirq())
				spin_unlock(&pool->lock);
			else
				spin_unlock_bh(&pool->lock);
			return;
		}
		if (in_serving_softirq())
			spin_unlock(&pool->lock);
		else
			spin_unlock_bh(&pool->lock);
                printk(KERN_ERR "dmapool full!\n");
		/*Should not happen!*/
	}
}
EXPORT_SYMBOL(dmapool_free_page);