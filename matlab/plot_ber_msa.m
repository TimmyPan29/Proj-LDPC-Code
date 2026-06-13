clear all ;close all ;clc

SNRH1 = 0:0.2:4.6; % Example SNR values
BER5070_0 = [1.52E-01	1.47E-01	1.40E-01	1.37E-01	1.31E-01	1.26E-01	1.25E-01	1.22E-01	1.19E-01	1.18E-01	1.14E-01	1.10E-01	1.09E-01	1.00E-01	9.81E-02	8.57E-02	7.18E-02	5.10E-02	2.58E-02	1.24E-02	4.05E-03	7.64E-04	1.65E-04	1.57E-05
]; % Example BER values

hold on ;
SNRH2 = 0:0.2:4.6;
BER10070_0 = [1.50E-01	1.44E-01	1.39E-01	1.37E-01	1.29E-01	1.28E-01	1.24E-01	1.22E-01	1.20E-01	1.18E-01	1.11E-01	1.10E-01	1.09E-01	1.05E-01	9.96E-02	9.05E-02	6.77E-02	5.39E-02	2.75E-02	1.23E-02	3.95E-03	9.75E-04	1.51E-04	1.76E-05
];

SNRH3 = 0:0.2:4.4;
BER20070_0 = [1.496E-01	1.439E-01	1.406E-01	1.375E-01	1.324E-01	1.310E-01	1.247E-01	1.222E-01	1.218E-01	1.168E-01	1.146E-01	1.122E-01	1.087E-01	1.086E-01	9.734E-02	9.226E-02	7.614E-02	4.691E-02	2.756E-02	1.293E-02	3.348E-03	9.958E-04	1.71E-04
];


semilogy(SNRH1, BER5070_0, '-o', 'LineWidth',1,'color','red');
semilogy(SNRH2, BER10070_0, '--+', 'LineWidth',1,'color','red');
semilogy(SNRH3, BER20070_0, '-*', 'LineWidth',1,'color','red');


xlabel('SNR (dB)');
ylabel('BER');
title('BER vs.SNR');
grid on;
legend('BER No.Error Block=50','BER No.Error Block=100','BER No.Error Block=200'); % Replace with appropriate labels
yticks([min(BER5070_0) : 0.025 : max(BER5070_0)]);
ytickformat('%.1e');
saveas(gcf, 'MSA_BER_ite70_diffrenterrorBlock.png');