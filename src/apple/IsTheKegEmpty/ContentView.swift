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
        VStack {
			if ApiClient.shared.loggedIn == true {
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
			else {
				VStack(alignment: .center) {
					Group() {
						Text("Email")
							.bold()
						TextField("Email", text: self.$email)
							.foregroundColor(self.colorScheme == .dark ? .white : .black)
							.background(self.colorScheme == .dark ? .black : .white)
							.autocorrectionDisabled()
					}
					.padding(EdgeInsets.init(top: 5, leading: 10, bottom: 5, trailing: 10))
					Group() {
						Text("Password")
							.bold()
						SecureField("Password", text: self.$password)
							.foregroundColor(self.colorScheme == .dark ? .white : .black)
							.background(self.colorScheme == .dark ? .black : .white)
							.bold()
					}
					.padding(EdgeInsets.init(top: 5, leading: 10, bottom: 20, trailing: 10))
					
					Group() {
						Button {
							if self.authVM.login(username: self.email, password: self.password) {
							}
							else {
								self.showingLoginError = true
							}
						} label: {
							Text("Login")
								.foregroundColor(self.colorScheme == .dark ? .black : .white)
								.fontWeight(Font.Weight.heavy)
								.frame(minWidth: 0, maxWidth: .infinity)
								.padding()
						}
						.alert("Login failed!", isPresented: self.$showingLoginError) { }
						.background(RoundedRectangle(cornerRadius: 10, style: .continuous))
						.opacity(0.8)
						.bold()
					}
					.padding(EdgeInsets.init(top: 5, leading: 10, bottom: 20, trailing: 10))
				}
				.background(
					Image("lock")
						.resizable()
						.edgesIgnoringSafeArea(.all)
						.aspectRatio(contentMode: .fill)
				)
			}
        }
		.padding(10)
    }
}

struct ContentView_Previews: PreviewProvider {
    static var previews: some View {
        ContentView()
    }
}
