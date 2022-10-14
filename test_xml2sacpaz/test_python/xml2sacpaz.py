#!/usr/bin/env python3
# 
# reads StationXML file from SeisComp3 and writes SAC PoleZeros
#

# import xml.etree.ElementTree as ET

import sys
from obspy import read_inventory
from obspy import UTCDateTime

######
###### start program, get command-line args
######
nargs    = int(len(sys.argv)) - 1
progname = sys.argv.pop(0)

if( nargs == 0 ):
	print( f"{progname}: error need 1 or more args got {nargs}" )
	sys.exit()

for in_filename in sys.argv: 
	out_filename = in_filename + "_nl.xml"
	print( f"{progname}: input={in_filename}, output={out_filename}" )

###### read one-liner StationXML from SC3 and write back out to clean StationXML
###### Don't need this because obspy.read_inventory does this
######
	# tree = ET.parse( in_filename )
	# root = tree.getroot()
	# ET.indent(tree, '  ')
	## print(root)
	## print(root.tag)
	# my_name_space = root.tag.split('}')[0].strip('{')
	## print( my_name_space )
	# ET.register_namespace( "", my_name_space )
	# tree.write( out_filename , encoding="ISO-8859-1", xml_declaration=True, method='xml', default_namespace=None )

####### read StationXML 
#######
	inv = read_inventory( in_filename, format='STATIONXML', level='response' )
	
	#### write new version that is clean, if needed
	inv.write( out_filename, format='STATIONXML', level='response' )

	NUMBER_INVENTORY = len(inv)
	print(f"number of inventories inv={NUMBER_INVENTORY}")
	
	for i_inv in range(0, NUMBER_INVENTORY, 1 ):
		net = inv[i_inv]
		NUMBER_NETS = len(net)
		print(f"number of networks inventory={NUMBER_NETS}")
		
		for i_net in range(0, NUMBER_NETS, 1 ):
			sta = net[i_net]
			NUMBER_STA = len(sta)
			print(f"number of stations inventory={NUMBER_STA}")

			for i_sta in range(0, NUMBER_STA, 1 ):
				# print( f"{i_inv} {i_net} {i_sta} ---------------------" )
				cha = sta[i_sta]
				# print(cha)
				resp = cha.response
				# print(resp)
				sacpz = resp.get_sacpz()
				# print(sacpz)

				my_net_code  = getattr( net, "code" )
				my_sta_code  = getattr( sta, "code" )
				my_chan_code = getattr( cha, "code" )
				my_loc_code  = getattr( cha, "location_code" )
				my_creation_date    = getattr( sta, "creation_date" ) 
				my_termination_date = getattr( sta, "termination_date" ) 

				start_time = my_creation_date.strftime( "%Y-%m-%dT%H%M%S" )
				if my_termination_date is None:
					stop = UTCDateTime(2049, 12, 31, 23, 59, 59 )
					stop_time = stop.strftime( "%Y-%m-%dT%H%M%S" )
				else:
					stop_time  = my_termination_date.strftime( "%Y-%m-%dT%H%M%S" )

				print( f"{my_net_code}.{my_sta_code}.{my_loc_code}.{my_chan_code} {my_creation_date} {my_termination_date} {start_time} {stop_time}" )

				inv0 = inv.select( station=my_sta_code, channel=my_chan_code )

				sacpaz_filename = "SAC_PZs_" + my_net_code + "_" + my_sta_code + "_" + my_chan_code + "_" + my_loc_code + "_" + start_time + "_" + stop_time
				resp_filename = my_net_code + "_" + my_sta_code + "_" + my_chan_code + "_" + my_loc_code + ".png"

				inv0.write(sacpaz_filename, format="SACPZ")

				#### optional bode plot
				# inv0.plot_response( min_freq=0.001, output="VEL", unwrap_phase=True, plot_degrees=True, show=False, outfile=resp_filename, label_epoch_dates=True )
				#
				#### optional fap
				# fap = resp.get_evalresp_response( 0.1, 1024, output="VEL" )
				# print(fap)
