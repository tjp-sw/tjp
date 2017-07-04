# sends a control message to the mega
def send_music(node, command, field3, channels, tracks):
	ctrl_msg= ';'.join([node,command,field3, ','.join(channels),','.join(tracks)
	print(ctrl_msg)
	# ***Magical message sending. 
	
def 