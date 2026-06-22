//Rotem Beker 217386598

package main

import (
	"math/rand"
	"sync"
	"time"
)

// DeliveryZone processes orders with token-limited concurrency
func DeliveryZone(zoneID int, zoneChan <-chan Order, tokens chan struct{}, seedB int64, events chan<- Event, wg *sync.WaitGroup) {
	defer wg.Done()

	// Create zone-specific RNG as specified
	rng := rand.New(rand.NewSource(seedB + int64(zoneID)*2000003))

	var orderWg sync.WaitGroup

	for order := range zoneChan {
		// Acquire token (blocks if no tokens available)
		tokens <- struct{}{}

		// Get the delay before spawning goroutine to ensure deterministic order
		delay := time.Duration(rng.Intn(21)) * time.Millisecond

		orderWg.Add(1)
		go func(o Order, d time.Duration) {
			defer orderWg.Done()

			// Send STARTED event
			events <- Event{
				Kind:    "STARTED",
				OrderID: o.OrderID,
				Zone:    zoneID,
			}

			// Process with deterministic delay
			time.Sleep(d)

			// Send COMPLETED event
			events <- Event{
				Kind:    "COMPLETED",
				OrderID: o.OrderID,
				Zone:    zoneID,
			}

			// Release token
			<-tokens
		}(order, delay)
	}

	// Wait for all orders in this zone to complete
	orderWg.Wait()
}
