//
//  ContentView.swift
//  Created by Michael Simms on 6/29/23.
//

import SwiftUI

struct ContentView: View {
	@Environment(\.colorScheme) var colorScheme

	@ObservedObject private var authVM : AuthVM = AuthVM()
	@ObservedObject private var inventoryVM : InventoryVM = InventoryVM()

	@State private var email: String = ""
	@State private var password: String = ""
	@State private var showingLoginError: Bool = false

	var body: some View {
		if ApiClient.shared.loginStatus == LoginStatus.LOGIN_STATUS_SUCCESS {
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
		else {
			VStack(alignment: .center) {
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

				Text("Password")
					.bold()
					.font(Font.system(size: 36, design: .default))
				SecureField("Password", text: self.$password)
					.foregroundColor(self.colorScheme == .dark ? .white : .black)
					.background(self.colorScheme == .dark ? .black : .white)
					.bold()
					.padding()
					.font(Font.system(size: 36, design: .default))

				Button {
					if !self.authVM.login(username: self.email, password: self.password) {
						self.showingLoginError = true
					}
				} label: {
					Text("Login")
						.foregroundColor(self.colorScheme == .dark ? .black : .white)
						.fontWeight(Font.Weight.heavy)
						.frame(minWidth: 0, maxWidth: .infinity)
						.padding()
						.font(Font.system(size: 36, design: .default))
				}
				.alert("Login failed!", isPresented: self.$showingLoginError) { }
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
