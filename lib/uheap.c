#include <inc/lib.h>

//==================================================================================//
//============================== GIVEN FUNCTIONS ===================================//
//==================================================================================//

int FirstTimeFlag = 1;
void InitializeUHeap()
{
	if(FirstTimeFlag)
	{
		initialize_dyn_block_system();
		cprintf("DYNAMIC BLOCK SYSTEM IS INITIALIZED\n");
#if UHP_USE_BUDDY
		initialize_buddy();
		cprintf("BUDDY SYSTEM IS INITIALIZED\n");
#endif
		FirstTimeFlag = 0;
	}
}

//==================================================================================//
//============================ REQUIRED FUNCTIONS ==================================//
//==================================================================================//

//=================================
// [1] INITIALIZE DYNAMIC ALLOCATOR:
//=================================
void initialize_dyn_block_system()
{
	//TODO: [PROJECT MS3] [USER HEAP - USER SIDE] initialize_dyn_block_system
	// your code is here, remove the panic and write your code
//	panic("initialize_dyn_block_system() is not implemented yet...!!");

	//[1] Initialize two lists (AllocMemBlocksList & FreeMemBlocksList) [Hint: use LIST_INIT()]
	LIST_INIT(&AllocMemBlocksList);
	LIST_INIT(&FreeMemBlocksList);

	//[2] Dynamically allocate the array of MemBlockNodes at VA USER_DYN_BLKS_ARRAY
	//	  (remember to set MAX_MEM_BLOCK_CNT with the chosen size of the array)
	MAX_MEM_BLOCK_CNT = NUM_OF_UHEAP_PAGES;
	uint32 start = ROUNDDOWN(USER_DYN_BLKS_ARRAY , PAGE_SIZE);
	uint32 	memsSize = ROUNDUP(MAX_MEM_BLOCK_CNT * sizeof(struct MemBlock) , PAGE_SIZE);
	sys_allocate_chunk(start,memsSize, PERM_WRITEABLE | PERM_USER);
	MemBlockNodes = (struct MemBlock *) start;

	//[3] Initialize AvailableMemBlocksList by filling it with the MemBlockNodes
	initialize_MemBlocksList(MAX_MEM_BLOCK_CNT);

	//[4] Insert a new MemBlock with the heap size into the FreeMemBlocksList
	struct MemBlock* freeMemBlock = LIST_FIRST(&AvailableMemBlocksList);
	freeMemBlock->size = USER_HEAP_MAX - USER_HEAP_START;
	freeMemBlock->sva = USER_HEAP_START;
	LIST_REMOVE(&AvailableMemBlocksList , freeMemBlock);
	LIST_INSERT_HEAD(&FreeMemBlocksList , freeMemBlock);
//	USER_HEAP_START-USER_DYN_BLKS_ARRAY  -
}

//=================================
// [2] ALLOCATE SPACE IN USER HEAP:
//=================================

void* malloc(uint32 size)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	if (size == 0) return NULL ;
	//==============================================================
	//==============================================================

	//TODO: [PROJECT MS3] [USER HEAP - USER SIDE] malloc
	// your code is here, remove the panic and write your code
	//panic("malloc() is not implemented yet...!!");

	// Steps:
	//	1) Implement FF strategy to search the heap for suitable space
	//		to the required allocation size (space should be on 4 KB BOUNDARY)
	//	2) if no suitable space found, return NULL
	// 	3) Return pointer containing the virtual address of allocated space,
	//
	//Use sys_isUHeapPlacementStrategyFIRSTFIT()... to check the current strategy
	struct MemBlock* block =NULL;
	uint32 sizeAlligned = ROUNDUP(size, PAGE_SIZE);
	if(sys_isUHeapPlacementStrategyFIRSTFIT()){
		block = alloc_block_FF(sizeAlligned);
		if(block != NULL)
		{
			insert_sorted_allocList(block);
			return (void*)block->sva;
		}
		else
		{
			return NULL;
		}
	}
	return NULL;
}

//=================================
// [3] FREE SPACE FROM USER HEAP:
//=================================
// free():
//	This function frees the allocation of the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	FROM main memory AND free pages from page file then switch back to the user again.
//
//	We can use sys_free_user_mem(uint32 virtual_address, uint32 size); which
//		switches to the kernel mode, calls free_user_mem() in
//		"kern/mem/chunk_operations.c", then switch back to the user mode here
//	the free_user_mem function is empty, make sure to implement it.
void free(void* virtual_address)
{
	//TODO: [PROJECT MS3] [USER HEAP - USER SIDE] free
	// your code is here, remove the panic and write your code
	//panic("free() is not implemented yet...!!");

	//you should get the size of the given allocation using its address
	//you need to call sys_free_user_mem()
	//refer to the project presentation and documentation for details
	struct MemBlock* memBlock = find_block(&AllocMemBlocksList , (uint32)  virtual_address);
	//if block is null it's already free -- break (for last 25% eval)
	if(memBlock == NULL)
		return ;

	//looping counter wise allocate chunk
	uint32 start = ROUNDDOWN((uint32)virtual_address , PAGE_SIZE);
	sys_free_user_mem(start ,ROUNDUP(memBlock->size, PAGE_SIZE));

	//removing block from allocated blocks and
	//inserting it with compaction into free list
	LIST_REMOVE(&AllocMemBlocksList , memBlock);
	insert_sorted_with_merge_freeList(memBlock);
}


//=================================
// [4] ALLOCATE SHARED VARIABLE:
//=================================
void* smalloc(char *sharedVarName, uint32 size, uint8 isWritable)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	if (size == 0) return NULL ;
	//==============================================================

	//TODO: [PROJECT MS3] [SHARING - USER SIDE] smalloc()
	// Write your code here, remove the panic and write your code
	//panic("smalloc() is not implemented yet...!!");
	// Steps:
	//	1) Implement FIRST FIT strategy to search the heap for suitable space
	//		to the required allocation size (space should be on 4 KB BOUNDARY)
	//	2) if no suitable space found, return NULL
	//	 Else,
	//	3) Call sys_createSharedObject(...) to invoke the Kernel for allocation of shared variable
	//		sys_createSharedObject(): if succeed, it returns the ID of the created variable. Else, it returns -ve
	//	4) If the Kernel successfully creates the shared variable, return its virtual address
	//	   Else, return NULL

	//This function should find the space of the required range
	// ******** ON 4KB BOUNDARY ******************* //

	//Use sys_isUHeapPlacementStrategyFIRSTFIT() to check the current strategy

	struct MemBlock* block =NULL;
		uint32 sizeAlligned = ROUNDUP(size, PAGE_SIZE);
		if(sys_isUHeapPlacementStrategyFIRSTFIT()){
			block = alloc_block_FF(sizeAlligned);
			if(block != NULL)
			{
				insert_sorted_allocList(block);
				int variableID = sys_createSharedObject(sharedVarName, size, isWritable, (void*)block->sva);
				if(variableID < 0){
					return NULL;
				}
				else{
					return (void*)block->sva;
				}
			}
			else
			{
				return NULL;
			}
		}
		return NULL;
}

//========================================
// [5] SHARE ON ALLOCATED SHARED VARIABLE:
//========================================
void* sget(int32 ownerEnvID, char *sharedVarName)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	//==============================================================

	//TODO: [PROJECT MS3] [SHARING - USER SIDE] sget()
	// Write your code here, remove the panic and write your code
	//panic("sget() is not implemented yet...!!");
	struct MemBlock* block =NULL;

	uint32 sizeOfSharedObject = ROUNDUP(sys_getSizeOfSharedObject(ownerEnvID , sharedVarName), PAGE_SIZE );

	if (sizeOfSharedObject == E_SHARED_MEM_NOT_EXISTS){
		return NULL;
	}

	if(sys_isUHeapPlacementStrategyFIRSTFIT()){

		block = alloc_block_FF(sizeOfSharedObject);

		if (block == NULL){
			return NULL;
		}

		insert_sorted_allocList(block);
		int id = sys_getSharedObject(ownerEnvID , sharedVarName , (void*)block->sva);

		if(id == E_SHARED_MEM_NOT_EXISTS){
			return NULL;
		}
		else{
			return (void*)block->sva;
		}

	} else {
		return NULL;
	}

	// Steps:
	//	1) Get the size of the shared variable (use sys_getSizeOfSharedObject())
	//	2) If not exists, return NULL
	//	3) Implement FIRST FIT strategy to search the heap for suitable space
	//		to share the variable (should be on 4 KB BOUNDARY)
	//	4) if no suitable space found, return NULL
	//	 Else,
	//	5) Call sys_getSharedObject(...) to invoke the Kernel for sharing this variable
	//		sys_getSharedObject(): if succeed, it returns the ID of the shared variable. Else, it returns -ve
	//	6) If the Kernel successfully share the variable, return its virtual address
	//	   Else, return NULL
	//

	//This function should find the space for sharing the variable
	// ******** ON 4KB BOUNDARY ******************* //

	//Use sys_isUHeapPlacementStrategyFIRSTFIT() to check the current strategy
}


//==================================================================================//
//============================== BONUS FUNCTIONS ===================================//
//==================================================================================//


//=================================
// REALLOC USER SPACE:
//=================================
//	Attempts to resize the allocated space at "virtual_address" to "new_size" bytes,
//	possibly moving it in the heap.
//	If successful, returns the new virtual_address, in which case the old virtual_address must no longer be accessed.
//	On failure, returns a null pointer, and the old virtual_address remains valid.

//	A call with virtual_address = null is equivalent to malloc().
//	A call with new_size = zero is equivalent to free().

//  Hint: you may need to use the sys_move_user_mem(...)
//		which switches to the kernel mode, calls move_user_mem(...)
//		in "kern/mem/chunk_operations.c", then switch back to the user mode here
//	the move_user_mem() function is empty, make sure to implement it.
void *realloc(void *virtual_address, uint32 new_size)
{
	//==============================================================
	//DON'T CHANGE THIS CODE========================================
	InitializeUHeap();
	//==============================================================
	// [USER HEAP - USER SIDE] realloc
	// Write your code here, remove the panic and write your code
	panic("realloc() is not implemented yet...!!");
}


//=================================
// FREE SHARED VARIABLE:
//=================================
//	This function frees the shared variable at the given virtual_address
//	To do this, we need to switch to the kernel, free the pages AND "EMPTY" PAGE TABLES
//	from main memory then switch back to the user again.
//
//	use sys_freeSharedObject(...); which switches to the kernel mode,
//	calls freeSharedObject(...) in "shared_memory_manager.c", then switch back to the user mode here
//	the freeSharedObject() function is empty, make sure to implement it.

void sfree(void* virtual_address)
{
	//TODO: [PROJECT MS3 - BONUS] [SHARING - USER SIDE] sfree()

	// Write your code here, remove the panic and write your code
	panic("sfree() is not implemented yet...!!");
}




//==================================================================================//
//========================== MODIFICATION FUNCTIONS ================================//
//==================================================================================//
void expand(uint32 newSize)
{
	panic("Not Implemented");

}
void shrink(uint32 newSize)
{
	panic("Not Implemented");

}
void freeHeap(void* virtual_address)
{
	panic("Not Implemented");
}
