//Rotem Beker 217386598
package main

// Order represents a food order in the system
type Order struct {
	OrderID      int // unique in [0..N-1]
	RestaurantID int // in [0..R-1]
	FoodType     int // zone index in [0..Z-1]
}

// Event represents system events for logging
type Event struct {
	Kind         string // "CREATED" | "DISPATCHED" | "STARTED" | "COMPLETED" | "DONE"
	OrderID      int
	RestaurantID int
	Zone         int
	Total        int // used for DONE event
}
