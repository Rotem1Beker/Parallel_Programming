//Rotem Beker 217386598
package main

import (
	"flag"
	"strconv"
	"strings"
	"sync"
)

func main() {
	// Parse command-line arguments
	n := flag.Int("n", 0, "total number of orders")
	restaurants := flag.Int("restaurants", 0, "number of restaurants")
	zones := flag.Int("zones", 0, "number of delivery zones")
	tokensStr := flag.String("tokens", "", "tokens per zone (comma-separated)")
	seedA := flag.Int64("seedA", 0, "seed for order generation")
	seedB := flag.Int64("seedB", 0, "seed for processing delays")
	flag.Parse()

	N := *n
	R := *restaurants
	Z := *zones

	// Parse tokens string
	tokenParts := strings.Split(*tokensStr, ",")
	tokensCount := make([]int, Z)
	for i := 0; i < Z && i < len(tokenParts); i++ {
		tokensCount[i], _ = strconv.Atoi(tokenParts[i])
	}

	// Create events channel for logger
	events := make(chan Event, N*4+1)

	// Start logger goroutine
	var loggerDone sync.WaitGroup
	loggerDone.Add(1)
	go func() {
		defer loggerDone.Done()
		Logger(events)
	}()

	// Create order channel for restaurants to dispatcher
	orderChan := make(chan Order, N)

	// Create zone channels
	zoneChans := make([]chan Order, Z)
	for i := 0; i < Z; i++ {
		zoneChans[i] = make(chan Order, N)
	}

	// Create token channels for each zone
	tokens := make([]chan struct{}, Z)
	for i := 0; i < Z; i++ {
		tokens[i] = make(chan struct{}, tokensCount[i])
	}

	// Start zone workers
	var zoneWg sync.WaitGroup
	for i := 0; i < Z; i++ {
		zoneWg.Add(1)
		go DeliveryZone(i, zoneChans[i], tokens[i], *seedB, events, &zoneWg)
	}

	// Start dispatcher
	go Dispatcher(orderChan, zoneChans, events)

	// Start restaurant goroutines
	var restaurantWg sync.WaitGroup
	for rid := 0; rid < R; rid++ {
		restaurantWg.Add(1)
		go Restaurant(rid, N, R, Z, *seedA, orderChan, events, &restaurantWg)
	}

	// Wait for all restaurants to finish
	restaurantWg.Wait()

	// Close order channel to signal dispatcher
	close(orderChan)

	// Wait for all zones to finish processing
	zoneWg.Wait()

	// Send DONE event
	events <- Event{
		Kind:  "DONE",
		Total: N,
	}

	// Close events channel
	close(events)

	// Wait for logger to finish
	loggerDone.Wait()
}
