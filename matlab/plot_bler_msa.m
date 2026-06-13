clear all ;close all ;clc

SNRH1 = 0:0.2:4.6; % Example SNR values

BLER5070_0 = [1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	9.80E-01	8.77E-01	6.94E-01	5.00E-01	2.62E-01	1.29E-01	4.45E-02	8.24E-03	1.75E-03	1.78E-04

]; % Example BER values
hold on ;
SNRH2 = 0:0.2:4.6;

BLER10070_0 = [1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	9.62E-01	8.70E-01	6.71E-01	5.38E-01	2.82E-01	1.31E-01	4.21E-02	9.93E-03	1.70E-03	1.88E-04

];
SNRH3 = 0:0.2:4.4;

BLER20070_0 = [1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	9.95E-01	9.48E-01	9.01E-01	7.69E-01	4.85E-01	2.77E-01	1.34E-01	3.50E-02	1.02E-02	1.77E-03


];


semilogy(SNRH1, BLER5070_0, '-o', 'LineWidth',1,'color','b');
semilogy(SNRH2, BLER10070_0, '--+', 'LineWidth',1,'color','b');
semilogy(SNRH3, BLER20070_0, '-*', 'LineWidth',1,'color','b');

xlabel('SNR (dB)');
ylabel('BLER');
title('BLER vs.SNR');
grid on;
legend('BLER No.Error Block=50','BLER No.Error Block=100','BLER No.Error Block=200'); % Replace with appropriate labels
yticks([min(BLER5070_0) : 0.05 : max(BLER5070_0)]);
ytickformat('%.1e');
saveas(gcf, 'MSA_ite70_diffrenterrorBlock.png');