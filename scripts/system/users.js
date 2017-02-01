"use strict";

//
//  users.js
//
//  Created by Faye Li on 18 Jan 2017.
//  Copyright 2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//

(function() { // BEGIN LOCAL_SCOPE
    var USERS_URL = "https://hifi-content.s3.amazonaws.com/faye/tablet-dev/users.html";

    var FRIENDS_WINDOW_URL = "https://metaverse.highfidelity.com/user/friends";
    var FRIENDS_WINDOW_WIDTH = 290;
    var FRIENDS_WINDOW_HEIGHT = 500;
    var FRIENDS_WINDOW_TITLE = "Add/Remove Friends";

    // Initialise visibility based on global service
    var VISIBILITY_VALUES_SET = {};
    VISIBILITY_VALUES_SET["all"] = true;
    VISIBILITY_VALUES_SET["friends"] = true;
    VISIBILITY_VALUES_SET["none"] = true;
    var myVisibility;
    if (GlobalServices.findableBy in VISIBILITY_VALUES_SET) {
        myVisibility = GlobalServices.findableBy;
    } else {
        // default to friends if it can't be determined
        myVisibility = "friends";
        GlobalServices.findableBy = myVisibilty;
    }

    var tablet = Tablet.getTablet("com.highfidelity.interface.tablet.system");
    var button = tablet.addButton({
        icon: "icons/tablet-icons/people-i.svg",
        text: "Users"
    });

    function onClicked() {
        tablet.gotoWebScreen(USERS_URL);
    }

    function onWebEventReceived(event) {
        print("Script received a web event, its type is " + typeof event);
        if (typeof event === "string") {
            event = JSON.parse(event);
        }
        if (event.type === "ready") {
            // send username to html
            var myUsername = GlobalServices.username;
            var object = {
                "type": "user-info",
                "data": {
                    "username": myUsername,
                    "visibility": myVisibility
                }
            };
            print("sending user info: " + JSON.stringify(object));
            tablet.emitScriptEvent(JSON.stringify(object));
        }
        if (event.type === "manage-friends") {
            // open a web overlay to metaverse friends page
            var friendsWindow = new OverlayWebWindow({
                title: FRIENDS_WINDOW_TITLE,
                width: FRIENDS_WINDOW_WIDTH,
                height: FRIENDS_WINDOW_HEIGHT,
                visible: false
            });
            friendsWindow.setURL(FRIENDS_WINDOW_URL);
            friendsWindow.setVisible(true);
            friendsWindow.raise();
        }
    }

    button.clicked.connect(onClicked);
    tablet.webEventReceived.connect(onWebEventReceived);

    function cleanup() {
        button.clicked.disconnect(onClicked);
        tablet.removeButton(button);
    }

    Script.scriptEnding.connect(cleanup);
}()); // END LOCAL_SCOPE
