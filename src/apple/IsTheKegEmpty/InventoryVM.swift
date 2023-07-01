//
//  InventoryVM.swift
//  Created by Michael Simms on 6/29/23.
//

import Foundation

class Keg : Identifiable, Hashable, Equatable {
	var id: UUID = UUID()
	var name: String = ""
	var capacity: Double = 0.0
	var percentage: Double = 0.0

	/// Constructor
	init() {
	}
	init(json: Decodable) {
	}
	
	/// Hashable overrides
	func hash(into hasher: inout Hasher) {
		hasher.combine(self.id)
	}
	
	/// Equatable overrides
	static func == (lhs: Keg, rhs: Keg) -> Bool {
		return lhs.id == rhs.id
	}
}

class InventoryVM : ObservableObject {
	func listKegs() -> Array<Keg> {
		return []
	}
}
