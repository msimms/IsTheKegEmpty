//
//  AuthVM.swift
//  Created by Michael Simms on 6/29/23.
//

import Foundation

class AuthVM : ObservableObject {
	func login(username: String, password: String) -> Bool {
		return ApiClient.shared.login(username: username, password: password)
	}
	
	func createLogin(username: String, password1: String, password2: String, realname: String) -> Bool {
		return ApiClient.shared.createLogin(username: username, password1: password1, password2: password2, realname: realname)
	}
}
