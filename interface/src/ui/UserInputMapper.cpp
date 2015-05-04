//
//  UserInputMapper.cpp
//  interface/src/ui
//
//  Created by Sam Gateau on 4/27/15.
//  Copyright 2015 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "UserInputMapper.h"
#include <algorithm>

void KeyboardMouseDevice::update() {
    _axisStateMap.clear();

    // For touch event, we need to check that the last event is not too long ago
    // Maybe it's a Qt issue, but the touch event sequence (begin, update, end) is not always called properly
    // The following is a workaround to detect that the touch sequence is over in case we didn;t see the end event
    if (_isTouching) {
        const auto TOUCH_EVENT_MAXIMUM_WAIT = 100; //ms
        auto currentTime = _clock.now();
        auto sinceLastTouch = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - _lastTouchTime);
        if (sinceLastTouch.count() > TOUCH_EVENT_MAXIMUM_WAIT) {
            _isTouching = false;
        }
    }
}

void KeyboardMouseDevice::keyPressEvent(QKeyEvent* event) {
    auto input = makeInput((Qt::Key) event->key());
    auto result = _buttonPressedMap.insert(input.getChannel());
    if (!result.second) {
        // key pressed again ? without catching the release event ?
    }
}
void KeyboardMouseDevice::keyReleaseEvent(QKeyEvent* event) {
    auto input = makeInput((Qt::Key) event->key());
    _buttonPressedMap.erase(input.getChannel());
}

void KeyboardMouseDevice::mousePressEvent(QMouseEvent* event, unsigned int deviceID) {
    auto input = makeInput((Qt::MouseButton) event->button());
    auto result = _buttonPressedMap.insert(input.getChannel());
    if (!result.second) {
        // key pressed again ? without catching the release event ?
    }
    _lastCursor = event->pos();
}
void KeyboardMouseDevice::mouseReleaseEvent(QMouseEvent* event, unsigned int deviceID) {
    auto input = makeInput((Qt::MouseButton) event->button());
    _buttonPressedMap.erase(input.getChannel());
}
void KeyboardMouseDevice::mouseMoveEvent(QMouseEvent* event, unsigned int deviceID) {
    QPoint currentPos = event->pos();
    QPoint currentMove = currentPos - _lastCursor;

    _axisStateMap[makeInput(MOUSE_AXIS_X_POS).getChannel()] = (currentMove.x() > 0 ? currentMove.x() : 0.0f);
    _axisStateMap[makeInput(MOUSE_AXIS_X_NEG).getChannel()] = (currentMove.x() < 0 ? -currentMove.x() : 0.0f);
     // Y mouse is inverted positive is pointing up the screen
    _axisStateMap[makeInput(MOUSE_AXIS_Y_POS).getChannel()] = (currentMove.y() < 0 ? -currentMove.y() : 0.0f);
    _axisStateMap[makeInput(MOUSE_AXIS_Y_NEG).getChannel()] = (currentMove.y() > 0 ? currentMove.y() : 0.0f);

    _lastCursor = currentPos;
}
void KeyboardMouseDevice::wheelEvent(QWheelEvent* event) {
    auto currentMove = event->angleDelta() / 120.0f;

    _axisStateMap[makeInput(MOUSE_AXIS_WHEEL_X_POS).getChannel()] = (currentMove.x() > 0 ? currentMove.x() : 0.0f);
    _axisStateMap[makeInput(MOUSE_AXIS_WHEEL_X_NEG).getChannel()] = (currentMove.x() < 0 ? -currentMove.x() : 0.0f);
    _axisStateMap[makeInput(MOUSE_AXIS_WHEEL_Y_POS).getChannel()] = (currentMove.y() > 0 ? currentMove.y() : 0.0f);
    _axisStateMap[makeInput(MOUSE_AXIS_WHEEL_Y_NEG).getChannel()] = (currentMove.y() < 0 ? -currentMove.y() : 0.0f);
}

glm::vec2 KeyboardMouseDevice::evalAverageTouchPoints(const QList<QTouchEvent::TouchPoint>& points) const {
    glm::vec2 averagePoint(0.0f);
    if (points.count() > 0) {
        for (auto& point : points) {
            averagePoint += glm::vec2(point.pos().x(), point.pos().y()); 
        }
        averagePoint /= (float)(points.count());
    }
    return averagePoint;
}


void KeyboardMouseDevice::touchBeginEvent(const QTouchEvent* event) {
    _isTouching = event->touchPointStates().testFlag(Qt::TouchPointPressed);
    _lastTouch = evalAverageTouchPoints(event->touchPoints());
    _lastTouchTime = _clock.now();
}
void KeyboardMouseDevice::touchEndEvent(const QTouchEvent* event) {
    _isTouching = false;
    _lastTouch = evalAverageTouchPoints(event->touchPoints());
    _lastTouchTime = _clock.now();
}
void KeyboardMouseDevice::touchUpdateEvent(const QTouchEvent* event) {
    auto currentPos = evalAverageTouchPoints(event->touchPoints());
    _lastTouchTime = _clock.now();
    
    if (!_isTouching) {
        _isTouching = event->touchPointStates().testFlag(Qt::TouchPointPressed);
    } else {
        auto currentMove = currentPos - _lastTouch;
    
        _axisStateMap[makeInput(TOUCH_AXIS_X_POS).getChannel()] = (currentMove.x > 0 ? currentMove.x : 0.0f);
        _axisStateMap[makeInput(TOUCH_AXIS_X_NEG).getChannel()] = (currentMove.x < 0 ? -currentMove.x : 0.0f);
        // Y mouse is inverted positive is pointing up the screen
        _axisStateMap[makeInput(TOUCH_AXIS_Y_POS).getChannel()] = (currentMove.y < 0 ? -currentMove.y : 0.0f);
        _axisStateMap[makeInput(TOUCH_AXIS_Y_NEG).getChannel()] = (currentMove.y > 0 ? currentMove.y : 0.0f);
    }

    _lastTouch = currentPos;
}

UserInputMapper::Input KeyboardMouseDevice::makeInput(Qt::Key code) {
    return UserInputMapper::Input(DEFAULT_DESKTOP_DEVICE, code & KEYBOARD_MASK, UserInputMapper::ChannelType::BUTTON);
}
UserInputMapper::Input KeyboardMouseDevice::makeInput(Qt::MouseButton code) {
    switch (code) {
        case Qt::LeftButton:
            return UserInputMapper::Input(DEFAULT_DESKTOP_DEVICE, MOUSE_BUTTON_LEFT, UserInputMapper::ChannelType::BUTTON);
        case Qt::RightButton:
            return UserInputMapper::Input(DEFAULT_DESKTOP_DEVICE, MOUSE_BUTTON_RIGHT, UserInputMapper::ChannelType::BUTTON);
        case Qt::MiddleButton:
            return UserInputMapper::Input(DEFAULT_DESKTOP_DEVICE, MOUSE_BUTTON_MIDDLE, UserInputMapper::ChannelType::BUTTON);
        default:
            return UserInputMapper::Input();
    };
}
UserInputMapper::Input KeyboardMouseDevice::makeInput(KeyboardMouseDevice::MouseAxisChannel axis) {
    return UserInputMapper::Input(DEFAULT_DESKTOP_DEVICE, axis, UserInputMapper::ChannelType::AXIS);
}
UserInputMapper::Input KeyboardMouseDevice::makeInput(KeyboardMouseDevice::TouchAxisChannel axis) {
    return UserInputMapper::Input(DEFAULT_DESKTOP_DEVICE, axis, UserInputMapper::ChannelType::AXIS);
}
UserInputMapper::Input KeyboardMouseDevice::makeInput(KeyboardMouseDevice::TouchButtonChannel button) {
    return UserInputMapper::Input(DEFAULT_DESKTOP_DEVICE, button, UserInputMapper::ChannelType::BUTTON);
}

void KeyboardMouseDevice::registerToUserInputMapper(UserInputMapper& mapper) {
    auto proxy = UserInputMapper::DeviceProxy::Pointer(new UserInputMapper::DeviceProxy());
    proxy->getButton = [this] (const UserInputMapper::Input& input, int timestamp) -> bool { return this->getButton(input._channel); };
    proxy->getAxis = [this] (const UserInputMapper::Input& input, int timestamp) -> float { return this->getAxis(input._channel); };
    mapper.registerDevice(DEFAULT_DESKTOP_DEVICE, proxy); // HARDCODED!, the KeyboardMouseDevice always take the DeviceID = 1
}

void KeyboardMouseDevice::assignDefaultInputMapping(UserInputMapper& mapper) {
    const float BUTTON_MOVE_SPEED = 1.0f;
    const float BUTTON_YAW_SPEED = 0.75f;
    const float BUTTON_PITCH_SPEED = 0.5f;
    const float MOUSE_YAW_SPEED = 0.5f;
    const float MOUSE_PITCH_SPEED = 0.25f;
    const float TOUCH_YAW_SPEED = 0.5f;
    const float TOUCH_PITCH_SPEED = 0.25f;
    //const float BUTTON_BOOM_SPEED = 0.1f;
 
    // AWSD keys mapping
    mapper.addInputChannel(UserInputMapper::LONGITUDINAL_BACKWARD, makeInput(Qt::Key_S), BUTTON_MOVE_SPEED);
    mapper.addInputChannel(UserInputMapper::LONGITUDINAL_FORWARD, makeInput(Qt::Key_W), BUTTON_MOVE_SPEED);
    mapper.addInputChannel(UserInputMapper::LATERAL_LEFT, makeInput(Qt::Key_A), BUTTON_MOVE_SPEED);
    mapper.addInputChannel(UserInputMapper::LATERAL_RIGHT, makeInput(Qt::Key_D), BUTTON_MOVE_SPEED);
    mapper.addInputChannel(UserInputMapper::VERTICAL_DOWN, makeInput(Qt::Key_C), BUTTON_MOVE_SPEED);
    mapper.addInputChannel(UserInputMapper::VERTICAL_UP, makeInput(Qt::Key_E), BUTTON_MOVE_SPEED);

  //  mapper.addInputChannel(UserInputMapper::BOOM_IN, makeInput(Qt::Key_W), makeInput(Qt::Key_Shift), BUTTON_BOOM_SPEED);
  //  mapper.addInputChannel(UserInputMapper::BOOM_OUT, makeInput(Qt::Key_S), makeInput(Qt::Key_Shift), BUTTON_BOOM_SPEED);
    mapper.addInputChannel(UserInputMapper::YAW_LEFT, makeInput(Qt::Key_A), makeInput(Qt::Key_Shift), BUTTON_YAW_SPEED);
    mapper.addInputChannel(UserInputMapper::YAW_RIGHT, makeInput(Qt::Key_D), makeInput(Qt::Key_Shift), BUTTON_YAW_SPEED);
    mapper.addInputChannel(UserInputMapper::PITCH_DOWN, makeInput(Qt::Key_C), makeInput(Qt::Key_Shift), BUTTON_PITCH_SPEED);
    mapper.addInputChannel(UserInputMapper::PITCH_UP, makeInput(Qt::Key_E), makeInput(Qt::Key_Shift), BUTTON_PITCH_SPEED);

    // Arrow keys mapping
    mapper.addInputChannel(UserInputMapper::LONGITUDINAL_BACKWARD, makeInput(Qt::Key_Down), BUTTON_MOVE_SPEED);
    mapper.addInputChannel(UserInputMapper::LONGITUDINAL_FORWARD, makeInput(Qt::Key_Up), BUTTON_MOVE_SPEED);
    mapper.addInputChannel(UserInputMapper::LATERAL_LEFT, makeInput(Qt::Key_Left), BUTTON_MOVE_SPEED);
    mapper.addInputChannel(UserInputMapper::LATERAL_RIGHT, makeInput(Qt::Key_Right), BUTTON_MOVE_SPEED);
    mapper.addInputChannel(UserInputMapper::VERTICAL_DOWN, makeInput(Qt::Key_PageDown), BUTTON_MOVE_SPEED);
    mapper.addInputChannel(UserInputMapper::VERTICAL_UP, makeInput(Qt::Key_PageUp), BUTTON_MOVE_SPEED);

 //   mapper.addInputChannel(UserInputMapper::BOOM_IN, makeInput(Qt::Key_Up), makeInput(Qt::Key_Shift), BUTTON_BOOM_SPEED);
 //   mapper.addInputChannel(UserInputMapper::BOOM_OUT, makeInput(Qt::Key_Down), makeInput(Qt::Key_Shift), BUTTON_BOOM_SPEED);
    mapper.addInputChannel(UserInputMapper::YAW_LEFT, makeInput(Qt::Key_Left), makeInput(Qt::Key_Shift), BUTTON_YAW_SPEED);
    mapper.addInputChannel(UserInputMapper::YAW_RIGHT, makeInput(Qt::Key_Right), makeInput(Qt::Key_Shift), BUTTON_YAW_SPEED);
    mapper.addInputChannel(UserInputMapper::PITCH_DOWN, makeInput(Qt::Key_Down), makeInput(Qt::Key_Shift), BUTTON_PITCH_SPEED);
    mapper.addInputChannel(UserInputMapper::PITCH_UP, makeInput(Qt::Key_Up), makeInput(Qt::Key_Shift), BUTTON_PITCH_SPEED);

    // Mouse move
    mapper.addInputChannel(UserInputMapper::PITCH_DOWN, makeInput(MOUSE_AXIS_Y_NEG), makeInput(Qt::RightButton), MOUSE_PITCH_SPEED);
    mapper.addInputChannel(UserInputMapper::PITCH_UP, makeInput(MOUSE_AXIS_Y_POS), makeInput(Qt::RightButton), MOUSE_PITCH_SPEED);
    mapper.addInputChannel(UserInputMapper::YAW_LEFT, makeInput(MOUSE_AXIS_X_NEG), makeInput(Qt::RightButton), MOUSE_YAW_SPEED);
    mapper.addInputChannel(UserInputMapper::YAW_RIGHT, makeInput(MOUSE_AXIS_X_POS), makeInput(Qt::RightButton), MOUSE_YAW_SPEED);

    
#ifdef Q_OS_MAC
    // wheel event modifier on Mac collide with the touchpad scroll event
    mapper.addInputChannel(UserInputMapper::PITCH_DOWN, makeInput(TOUCH_AXIS_Y_NEG), TOUCH_PITCH_SPEED);
    mapper.addInputChannel(UserInputMapper::PITCH_UP, makeInput(TOUCH_AXIS_Y_POS), TOUCH_PITCH_SPEED);
    mapper.addInputChannel(UserInputMapper::YAW_LEFT, makeInput(TOUCH_AXIS_X_NEG), TOUCH_YAW_SPEED);
    mapper.addInputChannel(UserInputMapper::YAW_RIGHT, makeInput(TOUCH_AXIS_X_POS), TOUCH_YAW_SPEED);
#else
    // Touch pad yaw pitch
    mapper.addInputChannel(UserInputMapper::PITCH_DOWN, makeInput(TOUCH_AXIS_Y_NEG), TOUCH_PITCH_SPEED);
    mapper.addInputChannel(UserInputMapper::PITCH_UP, makeInput(TOUCH_AXIS_Y_POS), TOUCH_PITCH_SPEED);
    mapper.addInputChannel(UserInputMapper::YAW_LEFT, makeInput(TOUCH_AXIS_X_NEG), TOUCH_YAW_SPEED);
    mapper.addInputChannel(UserInputMapper::YAW_RIGHT, makeInput(TOUCH_AXIS_X_POS), TOUCH_YAW_SPEED);
    
    // Wheel move
    mapper.addInputChannel(UserInputMapper::BOOM_IN, makeInput(MOUSE_AXIS_WHEEL_Y_NEG), BUTTON_BOOM_SPEED);
    mapper.addInputChannel(UserInputMapper::BOOM_OUT, makeInput(MOUSE_AXIS_WHEEL_Y_POS), BUTTON_BOOM_SPEED);
    mapper.addInputChannel(UserInputMapper::LATERAL_LEFT, makeInput(MOUSE_AXIS_WHEEL_X_NEG), BUTTON_YAW_SPEED);
    mapper.addInputChannel(UserInputMapper::LATERAL_RIGHT, makeInput(MOUSE_AXIS_WHEEL_X_POS), BUTTON_YAW_SPEED);

#endif

}

float KeyboardMouseDevice::getButton(int channel) const {
    if (!_buttonPressedMap.empty()) {
        if (_buttonPressedMap.find(channel) != _buttonPressedMap.end()) {
            return 1.0f;
        } else {
            return 0.0f;
        }
    }
    return 0.0f;
}
float KeyboardMouseDevice::getAxis(int channel) const {
    auto axis = _axisStateMap.find(channel);
    if (axis != _axisStateMap.end()) {
        return (*axis).second;
    } else {
        return 0.0f;
    }
}


// UserInputMapper Class
bool UserInputMapper::registerDevice(uint16 deviceID, const DeviceProxy::Pointer& proxy){
    _registeredDevices[deviceID] = proxy;
    return true;
}

UserInputMapper::DeviceProxy::Pointer UserInputMapper::getDeviceProxy(const Input& input) {
    auto device = _registeredDevices.find(input.getDevice());
    if (device != _registeredDevices.end()) {
        return (device->second);
    } else {
        return DeviceProxy::Pointer();
    }
}
bool UserInputMapper::addInputChannel(Action action, const Input& input, float scale) {
    return addInputChannel(action, input, Input(), scale);
}

bool UserInputMapper::addInputChannel(Action action, const Input& input, const Input& modifier, float scale) {
    // Check that the device is registered
    if (!getDeviceProxy(input)) {
        qDebug() << "UserInputMapper::addInputChannel: The input comes from a device #" << input.getDevice() << "is unknown. no inputChannel mapped.";
        return false;
    }

    auto inputChannel = InputChannel(input, modifier, action, scale);

    // Insert or replace the input to modifiers
    if (inputChannel.hasModifier()) {
        auto& modifiers = _inputToModifiersMap[input.getID()];
        modifiers.push_back(inputChannel._modifier);
        std::sort(modifiers.begin(), modifiers.end());
    }

    // Now update the action To Inputs side of things
    _actionToInputsMap.insert(ActionToInputsMap::value_type(action, inputChannel));

    return true;
}

int UserInputMapper::addInputChannels(const InputChannels& channels) {
    int nbAdded = 0;
    for (auto& channel : channels) {
        nbAdded += addInputChannel(channel._action, channel._input, channel._modifier, channel._scale);
    }
    return nbAdded;
}

int UserInputMapper::getInputChannels(InputChannels& channels) const {
    for (auto& channel : _actionToInputsMap) {
        channels.push_back(channel.second);
    }

    return _actionToInputsMap.size();
}

void UserInputMapper::update(float deltaTime) {

    // Reset the axis state for next loop
    for (auto& channel : _actionStates) {
        channel = 0.0f;
    }

    int currentTimestamp = 0;

    for (auto& channelInput : _actionToInputsMap) {
        auto& inputMapping = channelInput.second;
        auto& inputID = inputMapping._input;
        bool enabled = true;
        
        // Check if this input channel has modifiers and collect the possibilities
        auto modifiersIt = _inputToModifiersMap.find(inputID.getID());
        if (modifiersIt != _inputToModifiersMap.end()) {
            Modifiers validModifiers;
            bool isActiveModifier = false;
            for (auto& modifier : modifiersIt->second) {
                auto deviceProxy = getDeviceProxy(modifier);
                if (deviceProxy->getButton(modifier, currentTimestamp)) {
                    validModifiers.push_back(modifier);
                    isActiveModifier |= (modifier.getID() == inputMapping._modifier.getID());
                }
            }
            enabled = (validModifiers.empty() && !inputMapping.hasModifier()) || isActiveModifier;
        }

        // if enabled: default input or all modifiers on
        if (enabled) {
            auto deviceProxy = getDeviceProxy(inputID);
            switch (inputMapping._input.getType()) {
                case ChannelType::BUTTON: {
                    _actionStates[channelInput.first] += inputMapping._scale * float(deviceProxy->getButton(inputID, currentTimestamp));// * deltaTime; // weight the impulse by the deltaTime
                    break;
                }
                case ChannelType::AXIS: {
                    _actionStates[channelInput.first] += inputMapping._scale * deviceProxy->getAxis(inputID, currentTimestamp);
                    break;
                }
                case ChannelType::JOINT: {
                    // _channelStates[channelInput.first].jointVal = deviceProxy->getJoint(inputID, currentTimestamp);
                    break;
                }
                default: {
                    break; //silence please
                }
            }
        } else{
            // Channel input not enabled
            enabled = false;
        }
    }

    // Scale all the channel step with the scale
    for (auto i = 0; i < NUM_ACTIONS; i++) {
        _actionStates[i] *= _actionScales[i];
    }
}


void UserInputMapper::assignDefaulActionScales() {
    _actionScales[LONGITUDINAL_BACKWARD] = 1.0f; // 1m per unit
    _actionScales[LONGITUDINAL_FORWARD] = 1.0f; // 1m per unit
    _actionScales[LATERAL_LEFT] = 1.0f; // 1m per unit
    _actionScales[LATERAL_RIGHT] = 1.0f; // 1m per unit
    _actionScales[VERTICAL_DOWN] = 1.0f; // 1m per unit
    _actionScales[VERTICAL_UP] = 1.0f; // 1m per unit
    _actionScales[YAW_LEFT] = 1.0f; // 1 degree per unit
    _actionScales[YAW_RIGHT] = 1.0f; // 1 degree per unit
    _actionScales[PITCH_DOWN] = 1.0f; // 1 degree per unit
    _actionScales[PITCH_UP] = 1.0f; // 1 degree per unit
    _actionScales[BOOM_IN] = 1.0f; // 1m per unit
    _actionScales[BOOM_OUT] = 1.0f; // 1m per unit
}
