//Rotem Beker 217386598
package main

import (
	"math/rand"
	"sync"
)

// Restaurant generates orders and sends them to the dispatcher
func Restaurant(rid int, N int, R int, Z int, seedA int64, orderChan chan<- Order, events chan<- Event, wg *sync.WaitGroup) {
	defer wg.Done()

	// Calculate order ID range for this restaurant
	startID := (rid * N) / R
	endID := ((rid + 1) * N) / R

	// Create RNG as specified
	rng := rand.New(rand.NewSource(seedA + int64(rid)*1000003))

	// Generate orders with IDs in [startID, endID)
	for orderID := startID; orderID < endID; orderID++ {
		// Determine food type (zone)
		zone := rng.Intn(Z)

		order := Order{
			OrderID:      orderID,
			RestaurantID: rid,
			FoodType:     zone,
		}

		// Send CREATED event
		events <- Event{
			Kind:         "CREATED",
			OrderID:      orderID,
			RestaurantID: rid,
			Zone:         zone,
		}

		// Send order to dispatcher
		orderChan <- order
	}
}
