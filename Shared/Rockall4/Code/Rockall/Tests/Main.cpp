
//                                        Ruler
//       1         2         3         4         5         6         7         8
//345678901234567890123456789012345678901234567890123456789012345678901234567890

    /********************************************************************/
    /*                                                                  */
    /*   The standard layout.                                           */
    /*                                                                  */
    /*   The standard layout for 'cpp' files in this code is as         */
    /*   follows:                                                       */
    /*                                                                  */
    /*      1. Include files.                                           */
    /*      2. Constants local to the class.                            */
    /*      3. Data structures local to the class.                      */
    /*      4. Data initializations.                                    */
    /*      5. Static functions.                                        */
    /*      6. Class functions.                                         */
    /*                                                                  */
    /*   The constructor is typically the first function, class         */
    /*   member functions appear in alphabetical order with the         */
    /*   destructor appearing at the end of the file.  Any section      */
    /*   or function this is not required is simply omitted.            */
    /*                                                                  */
    /********************************************************************/

#include "BlendedHeap.hpp"
#include "DebugHeap.hpp"
//#include "DynamicDebugHeap.hpp"
#include "FastHeap.hpp"
#include "PageHeap.hpp"
#include "SingleSizeHeap.hpp"
#include "SmallHeap.hpp"
#include "SmpHeap.hpp"
#include "ZoneHeap.hpp"

#include "Environment.hpp"
#include "Thread.hpp"

    /********************************************************************/
    /*                                                                  */
    /*   Constants local to the class.                                  */
    /*                                                                  */
    /*   The constants supplied here control various tests.             */
    /*                                                                  */
    /********************************************************************/

CONST SBIT32 MaxArray				  = 256;
CONST SBIT32 MaxMultiple			  = 100;
CONST SBIT32 MaxSize				  = (18 * 1024);
CONST SBIT32 TestStride				  = 512;
CONST SBIT32 Test1					  = 2;
CONST SBIT32 Test2					  = 5;
CONST SBIT32 Test3					  = 7;
CONST SBIT32 Test4					  = 21;

    /********************************************************************/
    /*                                                                  */
    /*   Static class initialization.                                   */
    /*                                                                  */
    /*   Static class initialization creates an initial instance of     */
    /*   the class.                                                     */
    /*                                                                  */
    /********************************************************************/

STATIC BLENDED_HEAP BlendedHeap( 1048576,false,true,true );
STATIC DEBUG_HEAP DebugHeap( 0,false,true,true,true,true );
//STATIC DYNAMIC_DEBUG_HEAP DynamicDebugHeap( 0,false,true,true,true,30,30,true );
STATIC FAST_HEAP FastHeap( 4194304,true,true,true );
STATIC PAGE_HEAP PageHeap( 0,false,true,true );
STATIC SMALL_HEAP SmallHeap( 0,false,true,true );
STATIC SMP_HEAP SmpHeap( 4194304,true,true,true );
STATIC ZONE_HEAP ZoneHeap( 4194304,true,true,true );

STATIC ENVIRONMENT Environment;
STATIC THREAD Threads;

STATIC ROCKALL_FRONT_END *MainHeapTests[] =
	{
	& BlendedHeap,
	& DebugHeap,
//	& DynamicDebugHeap,
	& FastHeap,
	& PageHeap,
	& SmallHeap,
	& SmpHeap,
//	& ZoneHeap
	};

STATIC ROCKALL_FRONT_END *CrossHeapTests[] =
	{
	& BlendedHeap,
	& FastHeap,
	& SmpHeap,
	& SmallHeap,
	& BlendedHeap,
	& FastHeap,
	& SmpHeap
	};

    /********************************************************************/
    /*                                                                  */
    /*   The test loop.                                                 */
    /*                                                                  */
    /*   Execute a series of tests.                                     */
    /*                                                                  */
    /********************************************************************/

VOID Tests( VOID *Parameter )
    {
	REGISTER SBIT32 Count1;
	REGISTER SBIT32 Count2;
	REGISTER SBIT32 Size;

	//
	//   Execute the tests sequence on each test heap
	//   and include some cross heap tests.
	//
	for 
			( 
			Count1=0;
			Count1 < (sizeof(MainHeapTests) / sizeof(ROCKALL_FRONT_END*));
			Count1 ++ 
			)
		{
		REGISTER ROCKALL_FRONT_END *TestHeap = MainHeapTests[ Count1 ];

		for ( Size=0;Size < MaxSize;Size += TestStride )
			{
			AUTO void *Array[ MaxArray ];
			AUTO int ActualAllocations;
			AUTO int ActualSize[ MaxArray ];
			AUTO int DetailsSize;
			AUTO bool Result;

			//
			//   Try to globally lock and unlock 
			//   the test heap.
			//
			TestHeap -> LockAll();

			TestHeap -> UnlockAll();

			//
			//   Allocate a multiple elements to
			//   test the multiple alllocation
			//   functionality.
			//
			Result =
				(
				TestHeap -> MultipleNew
					( 
					& ActualAllocations,
					Array,
					MaxMultiple,
					Size,
					& ActualSize[0],
					True 
					)
				);

			//
			//   Copy the actual size.
			//
			for ( Count2=1;Count2 < MaxMultiple;Count2 ++ )
				{ ActualSize[ Count2 ] = ActualSize[0]; }

			//
			//   Ensure the all elements appear to have
			//   been allocated.
			//
			if 
					(
					(! Result)
						||
					(ActualAllocations != MaxMultiple)
						||
					(ActualSize[0] < Size)
					)
				{ printf( "'MultipleNew' failed on size %d\n",Size ); }

			//
			//   Now allocate some more elements on at 
			//   a time to test the single allocation 
			//   funtionality.
			//
			for ( Count2=MaxMultiple;Count2 < MaxArray;Count2 ++ )
				{
				Array[ Count2 ] =
					(
					TestHeap -> New
						(
						Size,
						& ActualSize[ Count2 ],
						True 
						)
					);

				//
				//   Test each allocation.
				//
				if 
						( 
						( Array[ Count2 ] != NULL )
							&&
						(ActualSize[ Count2 ] >= Size)
						)
					{
					//
					//   Zero the allocation.
					//
					memset( Array[ Count2 ],0,Size );
					}
				else
					{ printf( "'New' failed on size %d\n",Size ); }
				}

			//
			//   Examine the size of one of the allocations
			//   to ensure that it seems reasonable.
			//
			if 
					( 
					(! TestHeap -> Details( Array[ Test1 ], & DetailsSize ))
						||
					(ActualSize[ Test1 ] != DetailsSize)
					)
				{ printf( "'Details' failed on size %d\n",Size ); }

			//
			//   Ensure the allocation address appears to
			//   be known to the memory allocator.
			//
			if ( ! TestHeap -> KnownArea( Array[ Test4 ] ) )
				{ printf( "'KnownArea' failed on size %d\n",Size ); }

			//
			//   Ensure a random address appears to
			//   be unknown to the memory allocator.
			//
			if ( TestHeap -> KnownArea( ((void*) GuardValue) ) )
				{ printf( "'KnownArea' did not fail on size %d\n",Size ); }

			//
			//   Ensure the allocator thinks the address
			//   is in use.
			//
			if 
					( 
					(! TestHeap -> Verify( Array[ Test2 ], & DetailsSize ))
						||
					(ActualSize[ Test2 ] != DetailsSize)
					)
				{ printf( "'Verify' failed on size %d\n",Size ); }

			//
			//   Resize an existing allocation to test
			//   this functionaility.
			//
			Array[ Test3 ] =
				(
				TestHeap -> Resize
					(
					Array[ Test3 ],
					(Size * 2),
					1,
					& ActualSize[ Test3 ],
					false,
					true
					)
				);

			//
			//   Ensure the resize seems to have worked.
			//
			if 
					(
					(Array[ Test3 ] != NULL)
						&&
					(ActualSize[ Test3 ] >= (Size * 2))
					)
				{
				//
				//   Zero the allocation.
				//
				memset( Array[ Test3 ],0,(Size * 2) );
				}
			else
				{ printf( "'Resize' failed on size %d\n",Size ); }


			//
			//   Resize an existing allocation to back
			//   to the original size.
			//
			Array[ Test3 ] =
				(
				TestHeap -> Resize
					(
					Array[ Test3 ],
					Size,
					1,
					& ActualSize[ Test3 ],
					false,
					true
					)
				);

			//
			//   Ensure the resize seems to have worked.
			//
			if 
					(
					(Array[ Test3 ] != NULL)
						&&
					(ActualSize[ Test3 ] >= Size)
					)
				{
				//
				//   Zero the allocation.
				//
				memset( Array[ Test3 ],0,Size );
				}
			else
				{ printf( "'Resize' failed on size %d\n",Size ); }

			//
			//   Delete a block of the original allocations
			//   to test this functionality.
			//
			if 
					(
					! TestHeap -> MultipleDelete
						( 
						(MaxArray - MaxMultiple),
						& Array[ MaxMultiple ]
						)
					)
				{ printf( "'MultipleDelete' failed on size %d\n",Size ); }

			//
			//   Delete the last few allocations one at a time
			//   to test this functionaility.
			//
			for ( Count2=0;Count2 < MaxMultiple;Count2 ++ )
				{
				if ( ! TestHeap -> Delete( Array[ Count2 ],Count2 ) )
					{ printf( "'Delete' failed on size %d\n",Size ); }
				}
#ifdef DISABLE_MULTIPLE_PROCESSORS

			//
			//   Ensure the allocator thinks the address
			//   is now free.
			//
			if ( TestHeap -> Verify( Array[ Test2 ], & DetailsSize ) )
				{ printf( "'Verify' failed on size %d\n",Size ); }
#endif

			//
			//   Try to truncate the heap.
			//
			if ( ! TestHeap -> Truncate() )
				{ printf( "'Truncate' failed\n" ); }
			}
		}

	//
	//   Execute the cross heap test sequence on each 
	//   test heap.
	//
	for 
			( 
			Count1=2;
			Count1 < (sizeof(CrossHeapTests) / sizeof(ROCKALL_FRONT_END*));
			Count1 ++ 
			)
		{
		for ( Size=0;Size < MaxSize;Size += TestStride )
			{
			REGISTER void *Address = 
				(CrossHeapTests[ (Count1-2) ] -> New( Size,NULL,true ));

			//
			//   We have just created an allocation on
			//   a heap.
			//
			if ( Address == NULL )
				{ printf( "Cross heap 'New' failed on size %d\n",Size ); }

			//
			//   Now try to resize this allocation from
			//   another heap.
			//
			Address =
				(
				CrossHeapTests[ (Count1-1) ] -> Resize
					( 
					Address,
					(Size*2),
					1,
					false,
					true 
					)
				);

			//
			//   Ensure the resize appears to have worked.
			//
			if ( Address == NULL )
				{ printf( "Cross heap 'Resize' failed on size %d\n",Size ); }

			//
			//   Finally delete the allocation on another
			//   heap.
			if ( ! CrossHeapTests[ Count1 ] -> Delete( Address ) )
				{ printf( "Cross heap 'Delete' failed on size %d\n",Size ); }
			}
		}
    }

    /********************************************************************/
    /*                                                                  */
    /*   Main program.                                                  */
    /*                                                                  */
    /*   The main program is the initial entry point for the system.    */
    /*                                                                  */
    /********************************************************************/

int _cdecl main( INT ArgC, CHAR *ArgV[] )
    {
	REGISTER SBIT32 Count;

	printf( "Start of tests ...\n" );

	{
	AUTO SINGLE_SIZE_HEAP<int> SingleSizeHeap;
	//
	//   Test the single sized heap.
	//
	SingleSizeHeap.Delete( SingleSizeHeap.New() );
	}

	//
	//   Start a thread for each CPU in the system.
	//
	for ( Count=0;Count < (Environment.NumberOfCpus());Count ++ )
		{
		Threads.StartThread
			( 
			((NEW_THREAD) Tests),
			NULL
			); 
		}

	//
	//   Wait for all threads to complete.
	//
	(VOID) Threads.WaitForThreads();
	
	//
	//   Delete each heap at the end of the run.
	//
	for 
			( 
			Count=0;
			Count < (sizeof(MainHeapTests) / sizeof(ROCKALL_FRONT_END*));
			Count ++ 
			)
		{
		REGISTER ROCKALL_FRONT_END *TestHeap = MainHeapTests[ Count ];

		//
		//   When all of the tests have finished then
		//   delete all the haeps.
		//
		TestHeap -> DeleteAll( true );
		TestHeap -> DeleteAll( false );
		}

	printf( "End of tests ...\n" );

	return 0;
    }

