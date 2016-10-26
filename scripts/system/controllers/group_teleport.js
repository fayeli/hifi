//
// group_teleport.js
//
// Created by Si Fi Faye Li on 4 Oct, 2016
//

(function(){ // BEGIN LOCAL_SCOPE
    print('running group teleport script');

    var MODEL_URL;
    // loads default model or saved model from user settings
    if (Settings.getValue("Group-Teleport-Model") === "") {
        MODEL_URL = "http://hifi-content.s3.amazonaws.com/alan/dev/group-teleport-default-rug.fbx";
        Settings.setValue("Group-Teleport-Model", MODEL_URL);
    } else {
        MODEL_URL = Settings.getValue("Group-Teleport-Model");
    }
    var RING_URL = "http://hifi-content.s3.amazonaws.com/alan/dev/group-teleport-ring.fbx";
    var inGroupTeleportMode = false;

    function setupMenu() {
        print("Group Teleport set up Menu");
        Menu.addMenuItem({
            menuName: "Settings",
            menuItemName: "Customize Group Teleport"
        });
    }

    function handleMenuEvent(menuItem) {
        if (menuItem === "Customize Group Teleport") {
            print("Customize Group Teleport Clicked");
            var prompt = Window.prompt("Customize the look of Group Teleport.\nYour 3D model will be used in place of the default rug.\n\nPlease Note:\n    - Your model must be a FBX model in 5 x 0.05 x 5.\n    - If not, it will be scaled to fit.\n\nEnter Model URL:", MODEL_URL);
            if (prompt) {
                Window.alert("Group Teleport model changed to: \n" + prompt);
                MODEL_URL = prompt;
                // saves model to user settings
                Settings.setValue("Group-Teleport-Model", MODEL_URL);
            }
        }
    }

    setupMenu();
    Menu.menuItemEvent.connect(handleMenuEvent);

    function ThumbPad(hand) {
        this.hand = hand;
        var _thisPad = this;
        this.buttonPress = function(value) {
            _thisPad.buttonValue = value;
        };
        this.down = function() {
            var down = _thisPad.buttonValue === 1 ? 1.0 : 0.0;
            return down;
        };
    }

    function Trigger(hand) {
        this.hand = hand;
        var _this = this;
        this.buttonPress = function(value) {
            _this.buttonValue = value;
        };
        this.down = function() {
            var down = _this.buttonValue === 1 ? 1.0 : 0.0;
            return down;
        };
    }

    function GroupTeleporter() {
        var _this = this;
        this.updateConnected = false;
        var sphereID;
        var modelID;
        var ringID;
        this.createRug = function() {

            // Teleport Blue Ring Visual
            var position = MyAvatar.position;
            properties = {
                type: "Model",
                name: "Group Teleport Blue Ring Visual",
                dimensions: {
                    x: 5.0,
                    y: 0.001,
                    z: 5.0
                },
                position: position,
                modelURL: RING_URL
            };
            ringID = Entities.addEntity(properties);


            // Rug Model
            position.y = MyAvatar.getJointPosition("RightToeBase").y + 0.025;
            var properties = {
                type: "Model",
                name: "Group Teleportation Rug",
                position: position,
                dimensions: {
                    x: 5.0,
                    y: 0.05,
                    z: 5.0
                },
                modelURL: MODEL_URL,
                shapeType: "simple-hull"
            };
            modelID = Entities.addEntity(properties);
            print("Teleport Model added, entityItemID: " + modelID);

            // Teleport Sphere
            position = MyAvatar.position;
            properties = {
                type: "Sphere",
                name: "Invisible Teleport Sphere",
                position: position,
                dimensions: {
                    x: 5.0,
                    y: 0.25,
                    z: 5.0
                },
                ignoreForCollisions: true,
                script: "https://s3-us-west-1.amazonaws.com/hifi-content/faye/rugEntityScript.js",
                visible: false
            };
            sphereID = Entities.addEntity(properties);
            print("Invisible Spherical Rug Entity added, entityItemID: " + sphereID);
        };

        this.enterGroupTeleportMode = function() {
            print('enter group teleport mode');
            inGroupTeleportMode = true;
            this.createRug();
            Script.update.connect(this.update);
            this.updateConnected = true;
        };

        this.exitGroupTeleportMode = function() {
            print('exit group teleport mode');
            inGroupTeleportMode = false;
            prevLocation = null;
            currLocation = null;
            if (this.updateConnected) {
                Script.update.disconnect(this.update);
            }
            this.updateConnected = false;
            Entities.deleteEntity(sphereID);
            Entities.deleteEntity(modelID);
            Entities.deleteEntity(ringID);
            teleported = false;
            teleportCounter = 0;
            beforePosition = null;
            afterPosition = null;
        };

        var prevLocation = null;
        var currLocation = null;
        var teleported = false;
        // The counter and threshold are used to address having "Smooth Arrival" in teleport
        // Smooth arrival made the avatar teleports in intervals. Its position changes in multi-steps (set to 6 now in teleport.js) spaced between the original position to the final destination 
        var teleportCounter = 0;
        var TELEPORT_THRESHOLD = 6;
        var beforePosition;
        var afterPosition;
        this.update = function() {
            // check leader avatar's position, when it teleports, send a message to the rug channel.
            
            currLocation = MyAvatar.position;
            if (prevLocation === null) {
                prevLocation = currLocation;
            }

            var d = Vec3.distance(currLocation, prevLocation);
            
            // when the distance > 2, it means the leader started teleporting
            if (teleportCounter === 0 && d >= 2) {
                // save the before teleport position
                beforePosition = prevLocation;
                teleportCounter = teleportCounter + 1;
                print('Teleporting, counter = ' + teleportCounter);
            }

            // after teleporting, the leader position stablelizes (d = 0) and we know leader reached destination
            if (teleportCounter > 0 && d === 0) {
                teleportCounter = teleportCounter + 1;
                print('Teleporting, counter = ' + teleportCounter);
                if (teleportCounter >= TELEPORT_THRESHOLD){
                    teleported = true;
                    print('Teleported');
                }
            }
            if (teleported) {
                // send leader's before and after teleport position to other avatars on rug
                afterPosition = currLocation;
                var object = {
                    before: beforePosition,
                    after: afterPosition
                };
                var message = JSON.stringify(object);
                var channel = 'Group-Teleport-' + sphereID;
                Messages.sendMessage(channel, message);
                print('Leader sending message: ' + message +' To Channel: ' + channel);
                _this.exitGroupTeleportMode();
            }
            // Comment out following line to test by walking
            prevLocation = currLocation;
        };
    }

    var leftPad = new ThumbPad('left');
    var rightPad = new ThumbPad('right');
    var leftTrigger = new Trigger('left');
    var rightTrigger = new Trigger('right');

    var mappingName, teleportMapping;

    function registerMappings() {
        mappingName = 'Hifi-Group-Teleporter-Dev-' + Math.random();
        teleportMapping = Controller.newMapping(mappingName);

        //Maapping to keyboard space bar for testing when no vive is available
        teleportMapping.from(Controller.Hardware.Keyboard.Space).to(function(value) {
            if (value===0) {
                return;
            }
            print('Group Teleport Debug: Clicked space bar');
            if (inGroupTeleportMode) {
                teleporter.exitGroupTeleportMode();
            } else {
                teleporter.enterGroupTeleportMode();
            }
        });

        teleportMapping.from(Controller.Standard.RT).peek().to(rightTrigger.buttonPress);
        teleportMapping.from(Controller.Standard.LT).peek().to(leftTrigger.buttonPress);
        teleportMapping.from(Controller.Standard.RightPrimaryThumb).peek().to(rightPad.buttonPress);
        teleportMapping.from(Controller.Standard.LeftPrimaryThumb).peek().to(leftPad.buttonPress);
        teleportMapping.from(rightPad.down).when(leftPad.down)
            .to(function(value) {
                if (value===0) {
                    return;
                }
                if (rightTrigger.down()) {
                    return;
                }
                if (leftTrigger.down()) {
                    return;
                }
                print('Group Teleport Debug: Clicked both thumbpad');
                if (inGroupTeleportMode) {
                    teleporter.exitGroupTeleportMode();
                } else {
                    teleporter.enterGroupTeleportMode();
                }
            });
    }

    var teleporter = new GroupTeleporter();
    registerMappings();
    Controller.enableMapping(mappingName);

    function cleanupMenu() {
        Menu.removeMenuItem("Settings", "Customize Group Teleport");
    }

    function cleanup() {
        print('group teleport script cleanup');
        Controller.disableMapping(mappingName);
        if (teleporter.updateConnected) {
            Script.update.disconnect(teleporter.update);
            teleporter.updateConnected = false;
        }
        cleanupMenu();
    }

    Script.scriptEnding.connect(cleanup);

}()); // END LOCAL_SCOPE