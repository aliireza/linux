#include <linux/dma-wrapper.h>

dma_addr_t __dma_wrapper_map_page(struct device *dev, struct page *page,
		size_t offset, size_t size, enum dma_data_direction dir, const char *func) {
	trace_printk("[map_page] %s: dev %s page %llx offset %lu size %lu dir %d\n", func, dev->init_name, page_to_phys(page), offset, size, dir);
	return dma_map_page(dev,page,offset,size,dir);
		}
EXPORT_SYMBOL(__dma_wrapper_map_page);

void __dma_wrapper_unmap_page(struct device *dev, dma_addr_t addr, size_t size,
		enum dma_data_direction dir, const char *func) {
	trace_printk("[unmap_page] %s: dev %s addr %llx size %lu dir %d\n", func, dev->init_name, addr, size, dir);
	dma_unmap_page(dev,addr,size,dir);			
		}
EXPORT_SYMBOL(__dma_wrapper_unmap_page);

dma_addr_t __dma_wrapper_map_single(struct device *dev, void *ptr,
		size_t size, enum dma_data_direction dir, const char *func) {
	trace_printk("[map_single] %s: dev %s ptr %llx size %lu dir %d\n", func, dev->init_name, ptr, size, dir);
	return dma_map_single(dev,ptr,size,dir);	
		}
EXPORT_SYMBOL(__dma_wrapper_map_single);

void __dma_wrapper_unmap_single(struct device *dev, dma_addr_t addr,
		size_t size, enum dma_data_direction dir, const char *func) {
	trace_printk("[unmap_single] %s: dev %s ptr %llx size %lu dir %d\n", func, dev->init_name, addr, size, dir);
	dma_unmap_single(dev,addr,size,dir);	
		}
EXPORT_SYMBOL(__dma_wrapper_unmap_single);

struct sk_buff *__dma_wrapper_alloc_skb(unsigned int size, gfp_t priority, int flags,
			    int node, const char *func) {
	trace_printk("[alloc_skb] %s: size %u node %d\n", func, size, node);
	return __alloc_skb(size,priority,flags,node);
			    }
EXPORT_SYMBOL(__dma_wrapper_alloc_skb);

struct page *__dma_wrapper_alloc_pages(gfp_t gfp_mask, unsigned int order,  const char *func) {
	trace_printk("[alloc_pages] %s: order %d\n", func, order);
	return alloc_pages(gfp_mask,order);	
}
EXPORT_SYMBOL(__dma_wrapper_alloc_pages);

void *__dma_wrapper_page_frag_alloc(struct page_frag_cache *nc,
			     unsigned int fragsz, gfp_t gfp_mask, const char *func) {
	trace_printk("[page_frag_alloc] %s: size %d\n", func, fragsz);
	return page_frag_alloc(nc,fragsz,gfp_mask);	
			     }
EXPORT_SYMBOL(__dma_wrapper_page_frag_alloc);

void __dma_wrapper_page_frag_free(void *addr, const char *func) {
	trace_printk("[page_frag_free] %s: add %llx\n", func, addr);
	page_frag_free(addr);		
}
EXPORT_SYMBOL(__dma_wrapper_page_frag_free);