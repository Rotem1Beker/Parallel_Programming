//Rotem Beker 217386598
package main

import "fmt"

// Logger is the single goroutine that prints all events to stdout
func Logger(events <-chan Event) {
	for event := range events {
		switch event.Kind {
		case "CREATED":
			fmt.Printf("CREATED order=<%d> restaurant=<%d> type=<%d>\n", event.OrderID, event.RestaurantID, event.Zone)
		case "DISPATCHED":
			fmt.Printf("DISPATCHED order=<%d> zone=<%d>\n", event.OrderID, event.Zone)
		case "STARTED":
			fmt.Printf("STARTED order=<%d> zone=<%d>\n", event.OrderID, event.Zone)
		case "COMPLETED":
			fmt.Printf("COMPLETED order=<%d> zone=<%d>\n", event.OrderID, event.Zone)
		case "DONE":
			fmt.Printf("DONE total=<%d>\n", event.Total)
		}
	}
}
