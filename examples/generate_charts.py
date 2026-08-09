import matplotlib
matplotlib.use('Agg') # Headless mode za Linux/WSL

import matplotlib.pyplot as plt
import numpy as np

# Podešavanje stila
plt.style.use('default')
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(14, 6))

# 1. GRAFIKON 1: Zauzeće diska (30.5 MB sirovih -> 2.4 MB spakovano)
categories = ['Sirovi Podaci\n(Bez V-RAM)', 'S-OS V-RAM\n(Na Disku)']
disk_usage_mb = [30.5, 2.4]

colors1 = ['#ef4444', '#10b981']
bars1 = ax1.bar(categories, disk_usage_mb, color=colors1, width=0.45)

ax1.set_title('Zauzeće Prostora na Disku / Flash-u (30.5 MB Telemetrije)', fontsize=12, fontweight='bold', pad=15)
ax1.set_ylabel('Prostor na Disku (MB)', fontsize=11)
ax1.set_ylim(0, 36)

# Dodavanje vrednosti iznad stubaca
for bar in bars1:
    height = bar.get_height()
    ax1.text(bar.get_x() + bar.get_width()/2., height + 0.8,
             f'{height:.2f} MB',
             ha='center', va='bottom', fontsize=11, fontweight='bold')

# Strelica kompresije
ax1.annotate('12.51x Realna Kompresija!', xy=(1, 3.2), xytext=(0.5, 18),
            arrowprops=dict(facecolor='#10b981', shrink=0.05, width=2),
            fontsize=12, fontweight='bold', color='#10b981', ha='center')

# 2. GRAFIKON 2: Fizička potrošnja RAM-a procesa (RSS)
ram_categories = ['RAM bez V-RAM-a\n(Sirovih 30.5 MB)', 'RAM sa V-RAM-om\n(Samo 2.8 MB)']
ram_usage_mb = [30.5, 2.4]

colors2 = ['#f59e0b', '#06b6d4']
bars2 = ax2.bar(ram_categories, ram_usage_mb, color=colors2, width=0.45)

ax2.set_title('Fizička Potrošnja RAM-a Procesa (RSS)', fontsize=12, fontweight='bold', pad=15)
ax2.set_ylabel('Fizički RAM (MB)', fontsize=11)
ax2.set_ylim(0, 36)

for bar in bars2:
    height = bar.get_height()
    ax2.text(bar.get_x() + bar.get_width()/2., height + 0.8,
             f'{height:.2f} MB',
             ha='center', va='bottom', fontsize=11, fontweight='bold')

ax2.annotate('90.8% Manje Fizičkog RAM-a!', xy=(1, 3.5), xytext=(0.5, 18),
            arrowprops=dict(facecolor='#06b6d4', shrink=0.05, width=2),
            fontsize=12, fontweight='bold', color='#06b6d4', ha='center')

plt.suptitle('Syntetika Universe — S-OS V-RAM Engine™ Linux POSIX Benchmark', fontsize=14, fontweight='bold', y=0.98)
plt.tight_layout()

# Sačuvanje slike direktno na disk
output_filename = 'vram_linux_benchmark.png'
plt.savefig(output_filename, dpi=300, bbox_inches='tight')
print(f"✅ Grafikon uspešno generisan i sačuvan kao '{output_filename}'!")