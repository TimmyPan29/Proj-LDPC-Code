clear all ;close all ;clc

SNRH1 = 0:0.2:3.6; % Example SNR values
BERH1 = [1.09E-01	1.03E-01	9.80E-02	9.56E-02	8.49E-02	8.16E-02	7.75E-02	7.40E-02	6.71E-02	5.98E-02	4.80E-02	4.22E-02	3.86E-02	2.54E-02	3.54E-03	1.77E-03	5.59E-04	1.86E-04	3.88E-05
]; % Example BER values
hold on ;
SNRH2 = 0:0.2:3.6;
BERH2 = [1.09E-01	1.05E-01	9.80E-02	9.53E-02	8.48E-02	8.18E-02	7.74E-02	7.41E-02	6.73E-02	5.98E-02	4.75E-02	4.19E-02	2.15E-02	1.08E-02	4.34E-03	1.49E-03	3.98E-04	1.12E-04	2.63E-05

];
SNRH3 = 0:0.2:3.6;
BERH3 = [1.09E-01	1.05E-01	9.81E-02	9.54E-02	8.49E-02	8.17E-02	7.75E-02	7.40E-02	6.72E-02	5.98E-02	4.70E-02	4.22E-02	2.48E-02	9.98E-03	4.13E-03	1.19E-03	2.77E-04	6.98E-05	1.60E-05

];

semilogy(SNRH1, BERH1, '-o', 'LineWidth',1,'color','red');
semilogy(SNRH2, BERH2, '--+', 'LineWidth',1,'color','red');
semilogy(SNRH3, BERH3, '-*', 'LineWidth',1,'color','red');

xlabel('SNR (dB)');
ylabel('BER');
title('BER vs.SNR');
grid on;
legend('No.Ite=50','No.Ite=100','No. Ite=150'); % Replace with appropriate labels
yticks([min(BERH1) : 0.025 : max(BERH1)]);
ytickformat('%.1e');
saveas(gcf, 'SPA_BER_fixedBLOCK_plot.png');