#ifndef _DMA_WRAPPER_
#define _DMA_WRAPPER_

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/dma-mapping.h>
#include <linux/skbuff.h>
#include <linux/gfp.h>

#define dma_wrapper_map_page(d, p, o, s, r) 		__dma_wrapper_map_page(d, p, o, s, r, __FUNCTION__)
#define dma_wrapper_unmap_page(d, a, s, r)		__dma_wrapper_unmap_page(d, a, s, r, __FUNCTION__)

#define dma_wrapper_map_single(d, a, s, r)		__dma_wrapper_map_single(d, a, s, r, __FUNCTION__)
#define dma_wrapper_unmap_single(d, a, s, r) 		__dma_wrapper_unmap_single(d, a, s, r, __FUNCTION__)

#define dma_wrapper_alloc_skb(s, p, f, n) 		__dma_wrapper_alloc_skb(s, p, f, n, __FUNCTION__)
#define dma_wrapper_alloc_pages(m, o) 			__dma_wrapper_alloc_pages(m, o, __FUNCTION__)
#define dma_wrapper_page_frag_alloc(n, s, m) 		__dma_wrapper_page_frag_alloc(n, s, m, __FUNCTION__)

#define dma_wrapper_page_frag_free(a) 			__dma_wrapper_page_frag_free(a, __FUNCTION__)


dma_addr_t __dma_wrapper_map_page(struct device *dev, struct page *page,
		size_t offset, size_t size, enum dma_data_direction dir, const char *func);

void __dma_wrapper_unmap_page(struct device *dev, dma_addr_t addr, size_t size,
		enum dma_data_direction dir, const char *func);

dma_addr_t __dma_wrapper_map_single(struct device *dev, void *ptr,
		size_t size, enum dma_data_direction dir, const char *func);

void __dma_wrapper_unmap_single(struct device *dev, dma_addr_t addr,
		size_t size, enum dma_data_direction dir, const char *func);

struct sk_buff *__dma_wrapper_alloc_skb(unsigned int size, gfp_t priority, int flags,
			    int node, const char *func);

struct page *__dma_wrapper_alloc_pages(gfp_t gfp_mask, unsigned int order, const char *func);

void *__dma_wrapper_page_frag_alloc(struct page_frag_cache *nc,
			     unsigned int fragsz, gfp_t gfp_mask, const char *func);

void __dma_wrapper_page_frag_free(void *addr, const char *func);

#endif /*_DMA_WRAPPER_*/