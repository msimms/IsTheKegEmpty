//
//  ApiClient.swift
//  Created by Michael Simms on 6/30/23.
//

import Foundation

let BROADCAST_PROTOCOL = "http"
let BROADCAST_HOSTNAME = "127.0.0.1:5000"

let REMOTE_API_LOGIN_URL = "api/1.0/login"
let REMOTE_API_CREATE_LOGIN_URL = "api/1.0/create_login"
let REMOTE_API_IS_LOGGED_IN_URL = "api/1.0/login_status"
let REMOTE_API_LOGOUT_URL = "api/1.0/logout"
let REMOTE_API_KEG_STATUS_URL = "api/1.0/keg_status"
let REMOTE_API_REGISTER_DEVICE_URL = "api/1.0/register_device"
let REMOTE_API_UPDATE_KEG_WEIGHT_URL = "api/1.0/update_keg_weight"

let KEY_NAME_URL = "URL"
let KEY_NAME_RESPONSE_CODE = "ResponseCode"
let KEY_NAME_RESPONSE_DATA = "ResponseData"

let NOTIFICATION_NAME_LOGIN_PROCESSED = "LoginProcessed" // The server responded to a login attempt
let NOTIFICATION_NAME_CREATE_LOGIN_PROCESSED = "CreateLoginProcessed" // The server responded to an attempt to create a new login
let NOTIFICATION_NAME_LOGIN_CHECKED = "LoginChecked" // The server is responding to a login request
let NOTIFICATION_NAME_LOGGED_OUT = "LogoutProcessed" // The server is responding to a logout request

let PARAM_USERNAME = "username"
let PARAM_PASSWORD = "password"
let PARAM_PASSWORD1 = "password1"
let PARAM_PASSWORD2 = "password2"
let PARAM_REALNAME = "realname"
let PARAM_SESSION_TOKEN = "session_token"
let PARAM_SESSION_EXPIRY = "session_expiry"

enum LoginStatus : Int {
	case LOGIN_STATUS_UNKNOWN = 0
	case LOGIN_STATUS_SUCCESS
	case LOGIN_STATUS_FAILURE
}

class ApiClient {
	static let shared = ApiClient()
	
	/// Singleton constructor
	private init() {
	}
	
	func makeRequest(url: String, method: String, data: Dictionary<String, Any>) -> Bool {

		do {
			var request = URLRequest(url: URL(string: url)!)
			request.timeoutInterval = 30.0
			request.allowsExpensiveNetworkAccess = true
			request.httpMethod = method
			
			if data.count > 0 {
				
				// POST method, put the dictionary in the HTTP body
				if method == "POST" {
					let jsonData = try JSONSerialization.data(withJSONObject: data, options: [])
					let text = String(data: jsonData, encoding: String.Encoding.ascii)!
					let postLength = String(format: "%lu", text.count)
					
					request.setValue(postLength, forHTTPHeaderField: "Content-Length")
					request.setValue("application/json", forHTTPHeaderField: "Content-Type")
					request.httpBody = text.data(using:.utf8)
				}
				
				// GET method, append the parameters to the URL.
				else if method == "GET" {
					var newUrl = url + "?"
					for datum in data {
						newUrl = newUrl + datum.key
						newUrl = newUrl + "="
						newUrl = newUrl + String(describing: datum.value)
					}
					request.url = URL(string: newUrl)
				}
			}
			
			let session = URLSession.shared
			let dataTask = session.dataTask(with: request) { responseData, responseCode, error in
				if let httpResponse = responseCode as? HTTPURLResponse {
					
					var downloadedData: Dictionary<String, Any> = [:]
					downloadedData[KEY_NAME_URL] = url
					downloadedData[KEY_NAME_RESPONSE_CODE] = httpResponse
					downloadedData[KEY_NAME_RESPONSE_DATA] = responseData
					
					// Handle anything related to authorization. Trigger the notification no matter what so that
					// we can display error messages, etc.
					if url.contains(REMOTE_API_IS_LOGGED_IN_URL) {
						let notification = Notification(name: Notification.Name(rawValue: NOTIFICATION_NAME_LOGIN_CHECKED), object: downloadedData)
						NotificationCenter.default.post(notification)
					}
					else if url.contains(REMOTE_API_LOGIN_URL) {
						let notification = Notification(name: Notification.Name(rawValue: NOTIFICATION_NAME_LOGIN_PROCESSED), object: downloadedData)
						NotificationCenter.default.post(notification)
					}
					else if url.contains(REMOTE_API_CREATE_LOGIN_URL) {
						let notification = Notification(name: Notification.Name(rawValue: NOTIFICATION_NAME_CREATE_LOGIN_PROCESSED), object: downloadedData)
						NotificationCenter.default.post(notification)
					}
					else if url.contains(REMOTE_API_LOGOUT_URL) {
						let notification = Notification(name: Notification.Name(rawValue: NOTIFICATION_NAME_LOGGED_OUT), object: downloadedData)
						NotificationCenter.default.post(notification)
					}
					
					// For non-auth checks, only call trigger the notifications if we get an HTTP Ok error code.
					else if httpResponse.statusCode == 200 {
					
					}
					else {
						NSLog("Error code received from the server for " + url)
					}
				}
			}
			
			dataTask.resume()
			return true
		}
		catch {
		}
		return false
	}
	
	func login(username: String, password: String) -> Bool {
		var postDict: Dictionary<String, String> = [:]
		postDict[PARAM_USERNAME] = username
		postDict[PARAM_PASSWORD] = password
		
		let urlStr = String(format: "%@://%@/%@", BROADCAST_PROTOCOL, BROADCAST_HOSTNAME, REMOTE_API_LOGIN_URL)
		return self.makeRequest(url: urlStr, method: "POST", data: postDict)
	}
	
	func createLogin(username: String, password1: String, password2: String, realname: String) -> Bool {
		var postDict: Dictionary<String, String> = [:]
		postDict[PARAM_USERNAME] = username
		postDict[PARAM_PASSWORD1] = password1
		postDict[PARAM_PASSWORD2] = password2
		postDict[PARAM_REALNAME] = realname
		
		let urlStr = String(format: "%@://%@/%@", BROADCAST_PROTOCOL, BROADCAST_HOSTNAME, REMOTE_API_CREATE_LOGIN_URL)
		return self.makeRequest(url: urlStr, method: "POST", data: postDict)
	}
	
	func isLoggedIn(token: String) -> Bool {
		let urlStr = String(format: "%@://%@/%@?%@=%@", BROADCAST_PROTOCOL, BROADCAST_HOSTNAME, REMOTE_API_IS_LOGGED_IN_URL, PARAM_SESSION_TOKEN, token)
		return self.makeRequest(url: urlStr, method: "GET", data: [:])
	}
	
	func logout() -> Bool {
		let urlStr = String(format: "%@://%@/%@", BROADCAST_PROTOCOL, BROADCAST_HOSTNAME, REMOTE_API_LOGOUT_URL)
		return self.makeRequest(url: urlStr, method: "POST", data: [:])
	}
}
