#include <snowdrop/snowdrop.h>

SdResult sdCreateBuffer(const SdBufferCreateInfo* createInfo, SdBuffer* buffer)
{
	SdBuffer_t* ptr = new SdBuffer_t();
	
	ptr->data      = new uint8_t[createInfo->size];
	ptr->size      = createInfo->size;
	ptr->stride    = createInfo->stride;
	ptr->usage     = createInfo->usage;
	ptr->indexType = createInfo->indexType;
	
	if (ptr->usage == SD_BUFFER_USAGE_INDEX)
	{
		switch (ptr->indexType)
		{
		case SD_INDEX_TYPE_16_BIT:
			ptr->stride = sizeof(uint16_t);
			break;
		case SD_INDEX_TYPE_32_BIT:
			ptr->stride = sizeof(uint32_t);
			break;
		}
	}

	*buffer = ptr;
	return SD_SUCCESS;
}

void sdDestroyBuffer(SdBuffer buffer)
{
	delete[] buffer->data;
	delete buffer;
}

void* sdAccessBuffer(SdBuffer buffer)
{
	return buffer->data;
}