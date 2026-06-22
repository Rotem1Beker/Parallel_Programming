//Rotem Beker 217386598
package main

// Dispatcher collects orders from restaurants (fan-in) and routes to zones (fan-out)
func Dispatcher(orderChan <-chan Order, zoneChans []chan Order, events chan<- Event) {
	// Read orders from the combined restaurant channel
	for order := range orderChan {
		// Send DISPATCHED event
		events <- Event{
			Kind:    "DISPATCHED",
			OrderID: order.OrderID,
			Zone:    order.FoodType,
		}

		// Route order to appropriate zone based on FoodType
		zoneChans[order.FoodType] <- order
	}

	// When orderChan is closed, close all zone channels
	for _, zoneChan := range zoneChans {
		close(zoneChan)
	}
}
