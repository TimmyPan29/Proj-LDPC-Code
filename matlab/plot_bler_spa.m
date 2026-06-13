clear all ;close all ;clc

SNRH1 = 0:0.2:3.6; % Example SNR values
BLER5070_0 = [1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	9.43E-01	8.20E-01	7.58E-01	5.00E-01	2.20E-01	6.94E-02	3.46E-02	1.02E-02	3.34E-03	7.64E-04



]; % Example BER values

hold on ;
SNRH2 = 0:0.2:3.6;
BLER10070_0 = [1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	9.43E-01	8.20E-01	7.46E-01	4.00E-01	1.93E-01	8.16E-02	2.78E-02	7.29E-03	2.03E-03	4.87E-04
];

SNRH3 = 0:0.2:3.6;
BLER20070_0 = [1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	1.00E+00	9.43E-01	8.06E-01	7.46E-01	4.72E-01	1.88E-01	7.91E-02	2.26E-02	5.34E-03	1.34E-03	2.87E-04
];


semilogy(SNRH1, BLER5070_0, '-o', 'LineWidth',1,'color','b');
semilogy(SNRH2, BLER10070_0, '--+', 'LineWidth',1,'color','b');
semilogy(SNRH3, BLER20070_0, '-*', 'LineWidth',1,'color','b');


xlabel('SNR (dB)');
ylabel('BLER');
title('BLER vs.SNR');
grid on;
legend('No.Ite=50','No.Ite=100','No. Ite=150'); % Replace with appropriate labels
yticks([min(BLER5070_0) : 0.05 : max(BLER5070_0)]);
ytickformat('%.1e');
saveas(gcf, 'SPA_BLER_FixedBlock_differentIte.png');