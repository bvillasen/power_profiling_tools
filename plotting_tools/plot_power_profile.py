import sys, os
import numpy as np
import matplotlib.pyplot as plt

def convert_time_to_sec( time ):
  h, m, s = [ float(x)  for x in time.split(':') ]
  t_secs = 3600*h + 60*m + s
  return t_secs

def load_power_profile_file( file_name, time_start=None ):
  print( f'Loading file: {file_name}')
  file = open( file_name, 'r' )
  text = file.readlines()
  n_lines = len(text)
  dates, times, time_sec, rsmi_power, cray_power = [], [], [], [], []
  power_all = []
  if time_start is not None:  sec_start = convert_time_to_sec( time_start)
  for line in text:
    if line[0] == "#": continue
    n_power = len(line) - 2
    line = line.split(' ')
    date = line[0]
    time = line[1]
    t_secs = convert_time_to_sec( time )
    if time_start is not None:
      if t_secs < sec_start: continue
    power = line[2:-1]
    power_l = [ float(p) for p in power]
    power_all.append( power_l)
    dates.append(date)
    times.append(time)
    time_sec.append(t_secs)
  return { 'date':dates, 'time':times, 'time_sec':np.array(time_sec), 'power':np.array(power_all) }

benchmarks_dir = '/mnt/c/Users/bvillase/work/benchmarks'
input_dir  = f'{benchmarks_dir}/cholla/hydro/1024_n8'
output_dir = f'{input_dir}/figures'

file_name = f'{input_dir}/power_profile.txt'
data_power = load_power_profile_file( file_name )

nrows, ncols = 1, 1
h_scale_factor = 0.7
figure_width  = 6 *ncols
figure_height = figure_width * h_scale_factor
font_size = 12
legend_size = 10
fig_text_size = 6
text_color = 'black'

n_figs = nrows * ncols
fig, ax = plt.subplots(nrows=nrows, ncols=ncols, figsize=(figure_width*ncols,figure_height*nrows))
plt.subplots_adjust( hspace = 0.15, wspace=0.15)

lw=.3
data = data_power
time = data['time_sec'] - data['time_sec'][0] 
power_node = data['power'].T

for power_gpu in power_node:
  ax.plot( time, power_gpu, lw=lw, ls='--')

ax.set_xlabel( 'Time [sec]', fontsize=font_size, color=text_color  )
ax.set_ylabel( 'Power [W]', fontsize=font_size, color=text_color  )

ax.set_xlim(0, time[-1])

figure_name = f'{output_dir}/cholla_hydro_nnodes1.png'
fig.savefig( figure_name, bbox_inches='tight', dpi=300, facecolor=fig.get_facecolor() )
print( f'Saved Figure: {figure_name}' )

