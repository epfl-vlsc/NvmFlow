# NvmFlow Checkers

## Pair checker
* ensures a location in memory and its sentinel location is not written within the same epoch

## Durability checker
* ensures that a selected field always point to a location that is on PM

## Log/libpmemobj checker
* ensures that locations are logged before doing modifications