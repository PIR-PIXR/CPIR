import matplotlib.pyplot as plt
from matplotlib.pyplot import text

CKGS_server = []
LMC_CKGS_server = []
CKGS_client = []
LMC_CKGS_client = []

BE_server = []
LMC_BE_server = []
BE_client = []
LMC_BE_client = []

BE2_server = []
LMC_BE2_server = []
BE2_client = []
LMC_BE2_client = []

#case4 or 5: t = k - 1 or 1, n = 2^10, m = 2^12 running k (3, 4, 5)

#Read data from text file
with open('case4.txt', 'r') as f:
    times = []
    k_and_nm = []
    for line in f:
        try:
            k_and_nm.append(int(line.strip()))
        except ValueError:
            try:
                times.append(float(line.strip()))
            except ValueError:
                pass

#LM-CKGS
for i in range(0, 16, 4):
	CKGS_server.append(times[i])
	i += 1
	CKGS_client.append(times[i])
	i += 1
	LMC_CKGS_server.append(times[i])
	i += 1
	LMC_CKGS_client.append(times[i])
#LM-BE*
for i in range(16, 32, 4):
	BE_server.append(times[i])
	i += 1
	BE_client.append(times[i])
	i += 1
	LMC_BE_server.append(times[i])
	i += 1
	LMC_BE_client.append(times[i])
#LM-BE**	
for i in range(32, 48, 4):
	BE2_server.append(times[i])
	i += 1
	BE2_client.append(times[i])
	i += 1
	LMC_BE2_server.append(times[i])
	i += 1
	LMC_BE2_client.append(times[i])

#Server side
x_label = ['3', '4', '5', '6']

bar_width = 0.25

bar_1_x = [i for i in range(len(CKGS_server))]
bar_2_x = [i + bar_width for i in range(len(BE_server))]
bar_3_x = [i + bar_width*2 for i in range(len(BE2_server))]

#Server
plt.bar(bar_1_x, CKGS_server, width=bar_width, bottom = LMC_CKGS_server, label='k-CKGS', color = 'black')
plt.bar(bar_2_x, BE_server, width=bar_width, label='BE* (t = k - 1)', bottom = LMC_BE_server, fill = False, hatch = '.')
plt.bar(bar_3_x, BE2_server, width=bar_width, label='BE** (t = 1)', bottom = LMC_BE2_server, fill = False, hatch = '...')

plt.bar(bar_1_x, LMC_CKGS_server, width=bar_width, label='LMC-related time', fill = False, hatch = '///')
plt.bar(bar_2_x, LMC_BE_server, width=bar_width, fill = False, hatch = '///')
plt.bar(bar_3_x, LMC_BE2_server, width=bar_width, fill = False, hatch = '///')

for i in range(len(CKGS_server)):
    value = CKGS_server[i] + LMC_CKGS_server[i] + 0.1
    plt.text(bar_1_x[i] - 0.05, value, 'k-CKGS', color = 'black', rotation=90, fontweight='bold')
    
    value = BE_server[i] + LMC_BE_server[i] + 0.1
    plt.text(bar_2_x[i] - 0.05, value, 'BE*', color = 'black', rotation=90, fontweight='bold')
    
    value = BE2_server[i] + LMC_BE2_server[i] + 0.1
    plt.text(bar_3_x[i] - 0.05, value, 'BE**', color = 'black', rotation=90, fontweight='bold')
    
plt.xlabel('k', weight='bold')
plt.ylabel('seconds', weight='bold')
plt.title('Server average time', weight='bold', size = 14)
plt.xticks(bar_2_x, x_label)

# remove top and right spines
plt.gca().spines['right'].set_visible(False)
plt.gca().spines['top'].set_visible(False)

plt.legend(prop={'weight':'bold'})
plt.show()

#Client
plt.bar(bar_1_x, CKGS_client, width=bar_width, bottom = LMC_CKGS_client, label='k-CKGS', color = 'black')
plt.bar(bar_2_x, BE_client, width=bar_width, label='BE* (t = k - 1)', bottom = LMC_BE_client, fill = False, hatch = '.')
plt.bar(bar_3_x, BE2_client, width=bar_width, label='BE** (t = 1)', bottom = LMC_BE2_client, fill = False, hatch = '...')

plt.bar(bar_1_x, LMC_CKGS_client, width=bar_width, label='LMC-related time', fill = False, hatch = '///')
plt.bar(bar_2_x, LMC_BE_client, width=bar_width, fill = False, hatch = '///')
plt.bar(bar_3_x, LMC_BE2_client, width=bar_width, fill = False, hatch = '///')

for i in range(len(CKGS_server)):
    value = CKGS_client[i] + LMC_CKGS_client[i] + 0.06
    plt.text(bar_1_x[i] - 0.05, value, 'k-CKGS', color = 'black', rotation=90, fontweight='bold')
    
    value = BE_client[i] + LMC_BE_client[i] + 0.06
    plt.text(bar_2_x[i] - 0.05, value, 'BE*', color = 'black', rotation=90, fontweight='bold')
    
    value = BE2_client[i] + LMC_BE2_client[i] + 0.06
    plt.text(bar_3_x[i] - 0.05, value, 'BE**', color = 'black', rotation=90, fontweight='bold')
    
plt.xlabel('k', weight='bold')
plt.ylabel('seconds', weight='bold')
plt.title('Client time', weight='bold', size = 14)
plt.xticks(bar_2_x, x_label)

# remove top and right spines
plt.gca().spines['right'].set_visible(False)
plt.gca().spines['top'].set_visible(False)

plt.legend(prop={'weight':'bold'})
plt.show()








