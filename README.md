# NvmFlow Checkers

## Pair checker
* ensures a location in memory and its sentinel location is not written within the same epoch

## Durability checker/Double flush
* ensures that a selected field always point to a location that is on PM

## Log/libpmemobj checker/Double Log
* ensures that locations are logged before doing modifications
