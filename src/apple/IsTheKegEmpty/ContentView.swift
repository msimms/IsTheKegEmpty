//
//  ContentView.swift
//  Created by Michael Simms on 6/29/23.
//

import SwiftUI

struct ContentView: View {
	@Environment(\.colorScheme) var colorScheme

	@ObservedObject private var authVM : AuthVM = AuthVM()
	@ObservedObject private var inventoryVM : InventoryVM = InventoryVM()

	@State private var realname: String = ""
	@State private var email: String = ""
	@State private var password: String = ""
	@State private var passwordConfirmation: String = ""
	@State private var showingFailedToSendLoginError: Bool = false
	@State private var showingFailedToSendCreateLoginError: Bool = false
	@State private var creatingNewLogin: Bool = false

	var body: some View {
		// Logged in, show the kegs.
		if self.authVM.loginStatus == LoginStatus.LOGIN_STATUS_SUCCESS {
			VStack(alignment: .center) {
				List(self.inventoryVM.listKegs(), id: \.self) { item in
					NavigationLink(destination: KegView()) {
						HStack() {
							Image(systemName: "mug")
								.frame(width: 32)
							VStack(alignment: .leading) {
							}
							.onAppear() {
							}
						}
					}
				}
				.listStyle(.plain)
			}
		}
		
		// Not logged in, show the login screen.
		else {
			VStack(alignment: .center) {
				// Real Name
				if self.creatingNewLogin == true {
					Text("Real Name")
						.bold()
						.font(Font.system(size: 36, design: .default))
					TextField("Real Name", text: self.$realname)
						.foregroundColor(self.colorScheme == .dark ? .white : .black)
						.background(self.colorScheme == .dark ? .black : .white)
						.bold()
						.autocorrectionDisabled()
						.padding()
						.font(Font.system(size: 36, design: .default))
				}

				// Email
				Text("Email")
					.bold()
					.font(Font.system(size: 36, design: .default))
				TextField("Email", text: self.$email)
					.foregroundColor(self.colorScheme == .dark ? .white : .black)
					.background(self.colorScheme == .dark ? .black : .white)
					.bold()
					.autocorrectionDisabled()
					.padding()
					.font(Font.system(size: 36, design: .default))

				// Password
				Text("Password")
					.bold()
					.font(Font.system(size: 36, design: .default))
				SecureField("Password", text: self.$password)
					.foregroundColor(self.colorScheme == .dark ? .white : .black)
					.background(self.colorScheme == .dark ? .black : .white)
					.bold()
					.padding()
					.font(Font.system(size: 36, design: .default))
					.onSubmit {
						let _ = self.authVM.login(username: self.email, password: self.password)
					}

				// Password Confirmation
				if self.creatingNewLogin == true {
					Text("Password Confirmation")
						.bold()
						.font(Font.system(size: 36, design: .default))
					SecureField("Password Confirmation", text: self.$passwordConfirmation)
						.foregroundColor(self.colorScheme == .dark ? .white : .black)
						.background(self.colorScheme == .dark ? .black : .white)
						.bold()
						.padding()
						.font(Font.system(size: 36, design: .default))
						.onSubmit {
							let _ = self.authVM.createLogin(username: self.email, password1: self.password, password2: self.passwordConfirmation, realname: self.realname)
						}
					let _ = self.authVM.login(username: self.email, password: self.password)
				}

				// Login button
				if self.creatingNewLogin == false {
					Button {
						if !self.authVM.login(username: self.email, password: self.password) {
							self.showingFailedToSendCreateLoginError = true
						}
					} label: {
						Text("Login")
							.foregroundColor(self.colorScheme == .dark ? .black : .white)
							.fontWeight(Font.Weight.heavy)
							.frame(minWidth: 0, maxWidth: .infinity)
							.padding()
							.font(Font.system(size: 36, design: .default))
					}
					.alert("Failed to send the login request!", isPresented: self.$showingFailedToSendCreateLoginError) { }
					.alert("Login failed!", isPresented: self.$authVM.loginErrorReceived) { }
					.bold()
					.background(RoundedRectangle(cornerRadius: 10, style: .continuous))
					.padding()
					.buttonStyle(PlainButtonStyle())
				}

				// Create Login button
				Button {
					if self.creatingNewLogin == true {
						if !self.authVM.createLogin(username: self.email, password1: self.password, password2: self.passwordConfirmation, realname: self.realname) {
						}
					}
					else {
						self.creatingNewLogin = true
					}
				} label: {
					Text("Create Login")
						.foregroundColor(self.colorScheme == .dark ? .black : .white)
						.fontWeight(Font.Weight.heavy)
						.frame(minWidth: 0, maxWidth: .infinity)
						.padding()
						.font(Font.system(size: 36, design: .default))
				}
				.bold()
				.background(RoundedRectangle(cornerRadius: 10, style: .continuous))
				.padding()
				.buttonStyle(PlainButtonStyle())
			}
			.background(
				Image("lock")
					.resizable()
					.edgesIgnoringSafeArea(.all)
					.aspectRatio(contentMode: .fill)
			)
		}
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
