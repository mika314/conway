extends Spatial

var connection;

# Called when the node enters the scene tree for the first time.
func _ready():
	print("ready")
	connection = StreamPeerTCP.new()
	var res = connection.connect_to_host("127.0.0.1", 42069)
	if res != OK:
		print("connect error ", res)
	$GridMap.set_cell_item(0, 0, 0, 0)

var wasConnected = false
var CAM_SPEED = 14
var maxX = 0
var maxY = 0
var nextTime = 0
var time = 0
# Called every frame. 'delta' is the elapsed time since the previous frame.
# warning-ignore:unused_argument
func _process(delta):
	var dir = Vector3()
	if Input.is_action_pressed("ui_right"):
		dir.x += 1
	if Input.is_action_pressed("ui_left"):
		dir.x -= 1
	if Input.is_action_pressed("ui_down"):
		dir.z += 1
	if Input.is_action_pressed("ui_up"):
		dir.z -= 1
	if Input.is_action_pressed("ui_page_down"):
		dir.y -= 1
	if Input.is_action_pressed("ui_page_up"):
		dir.y += 1
	dir = dir.rotated(Vector3(0, 1, 0), rotation.y)
	$Camera.translation += dir * CAM_SPEED * delta
	var cam = $Camera.translation
	cam.x += (maxX * 2 + 64 - cam.x) * 0.5 * delta
	cam.z += (maxY * 2 + 64 - cam.z) * 0.5 * delta
	$Camera.translation = cam
	var status = connection.get_status()
	if status == StreamPeerTCP.STATUS_ERROR:
		print("status error")

	if status != StreamPeerTCP.STATUS_CONNECTED:
		if wasConnected:
			print("reconnect")
			var res = connection.connect_to_host("127.0.0.1", 42069)
			if res != OK:
				print("connect error ", res)
			wasConnected = false
		return
	if !wasConnected:
		print("connected")
		wasConnected = true;

	if time > nextTime:
		nextTime += 0.25
		connection.put_32($Camera.translation.x / 2 - 32)
		connection.put_32($Camera.translation.z / 2 - 32)
		
	time += delta
	
	if connection.get_available_bytes() <= 64 * 64 / 8 + 4 + 4:
		return
	var tmpX = connection.get_32()
	var tmpY = connection.get_32()
	maxX = connection.get_32()
	maxY = connection.get_32()
	var buffer = connection.get_data(64 * 64 / 8)
	for xx in range(0, 63):
		for yy in range(0, 63):
# warning-ignore:integer_division
			if buffer[1][yy * 8 + xx / 8] & (1 << xx % 8) == 0:
				$GridMap.set_cell_item(xx, 0, yy, 0)
			else:
				$GridMap.set_cell_item(xx, 0, yy, 1)
	$GridMap.translation.x = tmpX * 2
	$GridMap.translation.z = tmpY * 2
