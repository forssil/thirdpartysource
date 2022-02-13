/**************************************************
*         Copyright 2014 GaoH Inc.
*         Author:  Gu Cheng
*         History: 10/15/2014
*         Content: The Fourier Inverse Transform
***************************************************/

#ifndef AECHALFDUPLEX_F2TTRANSFORMER_H_
#define AECHALFDUPLEX_F2TTRANSFORMER_H_

//window for smooth
static const float syn_hwin256[] = {
	0.000149421078453171f,
	0.000597595007177931f,
	0.00134425391964726f,
	0.00238895154954133f,
	0.00373106349747415f,
	0.00536978760418699f,
	0.00730414442998667f,
	0.00953297784014107f,
	0.0120549556958828f,
	0.0148685706506078f,
	0.0179721410507924f,
	0.0213638119410917f,
	0.0250415561730168f,
	0.0290031756165303f,
	0.0332463024738333f,
	0.0377684006945618f,
	0.0425667674915437f,
	0.0476385349562126f,
	0.0529806717727114f,
	0.0585899850296627f,
	0.0644631221285217f,
	0.0705965727873714f,
	0.0769866711389634f,
	0.0836295979217494f,
	0.0905213827625935f,
	0.097657906549802f,
	0.105034903895052f,
	0.112647965682748f,
	0.120492541705279f,
	0.128563943382607f,
	0.13685734656456f,
	0.145367794414147f,
	0.154090200370186f,
	0.163019351187458f,
	0.172149910052584f,
	0.181476419773753f,
	0.190993306042403f,
	0.200694880764894f,
	0.210575345462196f,
	0.220628794735546f,
	0.230849219796014f,
	0.24123051205586f,
	0.251766466779543f,
	0.262450786792195f,
	0.273277086243339f,
	0.284238894423618f,
	0.295329659632231f,
	0.306542753092784f,
	0.317871472915207f,
	0.329309048101366f,
	0.340848642591985f,
	0.352483359352449f,
	0.364206244495054f,
	0.376010291435238f,
	0.387888445079305f,
	0.399833606041146f,
	0.411838634885427f,
	0.423896356394721f,
	0.435999563858022f,
	0.448141023378083f,
	0.460313478195001f,
	0.472509653023471f,
	0.484722258401111f,
	0.496943995045254f,
	0.509167558215622f,
	0.521385642080249f,
	0.533590944082063f,
	0.545776169303514f,
	0.557934034826625f,
	0.570057274085884f,
	0.582138641211355f,
	0.594170915359414f,
	0.606146905028547f,
	0.618059452357584f,
	0.629901437403849f,
	0.641665782398637f,
	0.653345455977481f,
	0.664933477382692f,
	0.67642292063565f,
	0.687806918676346f,
	0.699078667467724f,
	0.710231430062342f,
	0.721258540628942f,
	0.73215340843651f,
	0.742909521793458f,
	0.753520451939554f,
	0.763979856888295f,
	0.774281485217411f,
	0.784419179805244f,
	0.794386881510759f,
	0.804178632795004f,
	0.813788581281829f,
	0.82321098325577f,
	0.832440207094966f,
	0.841470736637101f,
	0.850297174476321f,
	0.858914245189185f,
	0.867316798487695f,
	0.875499812297549f,
	0.883458395759753f,
	0.891187792153812f,
	0.898683381740745f,
	0.905940684524234f,
	0.912955362928245f,
	0.919723224389528f,
	0.926240223863451f,
	0.932502466241655f,
	0.938506208680101f,
	0.94424786283611f,
	0.949723997013056f,
	0.954931338211443f,
	0.959866774085119f,
	0.96452735480148f,
	0.96891029480454f,
	0.97301297447981f,
	0.976832941720003f,
	0.980367913390621f,
	0.983615776694547f,
	0.98657459043483f,
	0.98924258617491f,
	0.991618169295584f,
	0.993699919948085f,
	0.995486593902703f,
	0.99697712329244f,
	0.998170617251262f,
	0.99906636244655f,
	0.999663823505453f,
	0.999962643334866f,
	0.999962643334866f,
	0.999663823505453f,
	0.99906636244655f,
	0.998170617251262f,
	0.99697712329244f,
	0.995486593902703f,
	0.993699919948085f,
	0.991618169295584f,
	0.98924258617491f,
	0.98657459043483f,
	0.983615776694547f,
	0.980367913390621f,
	0.976832941720003f,
	0.97301297447981f,
	0.96891029480454f,
	0.96452735480148f,
	0.959866774085119f,
	0.954931338211443f,
	0.949723997013056f,
	0.94424786283611f,
	0.938506208680101f,
	0.932502466241655f,
	0.926240223863451f,
	0.919723224389528f,
	0.912955362928245f,
	0.905940684524234f,
	0.898683381740745f,
	0.891187792153812f,
	0.883458395759753f,
	0.875499812297549f,
	0.867316798487695f,
	0.858914245189185f,
	0.850297174476321f,
	0.841470736637101f,
	0.832440207094966f,
	0.82321098325577f,
	0.813788581281829f,
	0.804178632795004f,
	0.794386881510759f,
	0.784419179805244f,
	0.774281485217411f,
	0.763979856888295f,
	0.753520451939554f,
	0.742909521793458f,
	0.73215340843651f,
	0.721258540628942f,
	0.710231430062342f,
	0.699078667467724f,
	0.687806918676346f,
	0.67642292063565f,
	0.664933477382692f,
	0.653345455977481f,
	0.641665782398637f,
	0.629901437403849f,
	0.618059452357584f,
	0.606146905028547f,
	0.594170915359414f,
	0.582138641211355f,
	0.570057274085884f,
	0.557934034826625f,
	0.545776169303514f,
	0.533590944082063f,
	0.521385642080249f,
	0.509167558215622f,
	0.496943995045254f,
	0.484722258401111f,
	0.472509653023471f,
	0.460313478195001f,
	0.448141023378083f,
	0.435999563858022f,
	0.423896356394721f,
	0.411838634885427f,
	0.399833606041146f,
	0.387888445079305f,
	0.376010291435238f,
	0.364206244495054f,
	0.352483359352449f,
	0.340848642591985f,
	0.329309048101366f,
	0.317871472915207f,
	0.306542753092784f,
	0.295329659632231f,
	0.284238894423618f,
	0.273277086243339f,
	0.262450786792195f,
	0.251766466779543f,
	0.24123051205586f,
	0.230849219796014f,
	0.220628794735546f,
	0.210575345462196f,
	0.200694880764894f,
	0.190993306042403f,
	0.181476419773753f,
	0.172149910052584f,
	0.163019351187458f,
	0.154090200370186f,
	0.145367794414147f,
	0.13685734656456f,
	0.128563943382607f,
	0.120492541705279f,
	0.112647965682748f,
	0.105034903895052f,
	0.097657906549802f,
	0.0905213827625935f,
	0.0836295979217494f,
	0.0769866711389634f,
	0.0705965727873714f,
	0.0644631221285217f,
	0.0585899850296627f,
	0.0529806717727114f,
	0.0476385349562126f,
	0.0425667674915437f,
	0.0377684006945618f,
	0.0332463024738333f,
	0.0290031756165303f,
	0.0250415561730168f,
	0.0213638119410917f,
	0.0179721410507924f,
	0.0148685706506078f,
	0.0120549556958828f,
	0.00953297784014107f,
	0.00730414442998667f,
	0.00536978760418699f,
	0.00373106349747415f,
	0.00238895154954133f,
	0.00134425391964726f,
	0.000597595007177931f,
	0.000149421078453171f
};

static const float syn_hwin512[]= {
	3.750244528e-005f,  0.0001500041554f,  0.000337488254f,  0.0005999266166f,  0.0009372798749f,  0.001349497423f,  0.001836517423f,  0.002398266818f,  
	0.003034661341f,  0.003745605525f,  0.004530992723f,  0.005390705117f,  0.006324613744f,  0.007332578507f,  0.008414448203f,  0.009570060539f,  
	0.01079924216f,  0.01210180869f,  0.01347756471f,  0.01492630386f,  0.0164478088f,  0.01804185131f,  0.01970819225f,  0.02144658166f,  0.02325675877f,  
	0.02513845202f,  0.02709137915f,  0.0291152472f,  0.03120975257f,  0.03337458106f,  0.03560940792f,  0.03791389792f,  0.04028770535f,  0.04273047413f,  
	0.0452418378f,  0.04782141965f,  0.05046883271f,  0.05318367984f,  0.05596555379f,  0.05881403724f,  0.06172870291f,  0.06470911356f,  0.0677548221f,  
	0.07086537164f,  0.07404029558f,  0.07727911764f,  0.08058135196f,  0.08394650319f,  0.08737406651f,  0.09086352775f,  0.09441436347f,  0.098026041f,  
	0.1016980186f,  0.1054297453f,  0.1092206614f,  0.1130701983f,  0.1169777784f,  0.1209428156f,  0.1249647151f,  0.1290428736f,  0.1331766792f,  0.137365512f,  
	0.1416087434f,  0.1459057371f,  0.1502558483f,  0.1546584246f,  0.1591128055f,  0.1636183228f,  0.1681743007f,  0.1727800557f,  0.1774348968f,  0.1821381259f,  
	0.1868890374f,  0.1916869185f,  0.1965310497f,  0.2014207041f,  0.2063551484f,  0.2113336423f,  0.216355439f,  0.2214197851f,  0.2265259209f,  0.2316730806f,  
	0.2368604919f,  0.2420873767f,  0.247352951f,  0.2526564247f,  0.2579970025f,  0.263373883f,  0.2687862598f,  0.2742333208f,  0.2797142491f,  0.2852282225f, 
	0.2907744136f,  0.2963519907f,  0.301960117f,  0.3075979512f,  0.3132646475f,  0.318959356f,  0.3246812224f,  0.3304293883f,  0.3362029915f,  0.3420011658f, 
	0.3478230414f,  0.3536677451f,  0.3595344001f,  0.3654221263f,  0.3713300405f,  0.3772572564f,  0.383202885f,  0.3891660342f,  0.3951458097f,  0.4011413143f,  
	0.4071516486f,  0.4131759112f,  0.4192131982f,  0.425262604f,  0.4313232211f,  0.4373941404f,  0.4434744512f,  0.4495632414f,  0.4556595977f,  0.4617626054f,  
	0.467871349f,  0.4739849123f,  0.4801023782f,  0.4862228288f,  0.4923453462f,  0.4984690118f,  0.5045929071f,  0.5107161134f,  0.5168377122f,  0.5229567852f,  
	0.5290724145f,  0.5351836825f,  0.5412896727f,  0.5473894691f,  0.5534821565f,  0.5595668211f,  0.5656425501f,  0.5717084321f,  0.5777635571f,  0.5838070168f,  
	0.5898379047f,  0.595855316f,  0.601858348f,  0.6078461003f,  0.6138176746f,  0.6197721752f,  0.6257087088f,  0.6316263848f,  0.6375243156f,  0.6434016164f, 
	0.6492574055f,  0.6550908046f,  0.6609009385f,  0.6666869357f,  0.6724479283f,  0.678183052f,  0.6838914464f,  0.6895722554f,  0.6952246267f,  0.7008477123f, 
	0.7064406689f,  0.7120026573f,  0.7175328432f,  0.7230303971f,  0.7284944943f,  0.733924315f,  0.7393190448f,  0.7446778745f,  0.75f,  0.7552846231f,  0.760530951f, 
	0.7657381967f,  0.7709055791f,  0.776032323f,  0.7811176593f,  0.7861608253f,  0.7911610643f,  0.7961176263f,  0.8010297678f,  0.8058967519f,  0.8107178484f,  
	0.8154923343f,  0.8202194932f,  0.8248986161f,  0.8295290009f,  0.8341099533f,  0.8386407858f,  0.8431208189f,  0.8475493806f,  0.8519258064f,  0.8562494399f,  
	0.8605196326f,  0.8647357437f,  0.8688971409f,  0.8730032f,  0.8770533049f,  0.8810468481f,  0.8849832306f,  0.8888618618f,  0.89268216f,  0.8964435519f,  
	0.9001454735f,  0.9037873693f,  0.9073686931f,  0.9108889076f,  0.9143474847f,  0.9177439057f,  0.921077661f,  0.9243482505f,  0.9275551836f,  0.9306979793f,  
	0.9337761661f,  0.9367892822f,  0.9397368756f,  0.9426185042f,  0.9454337357f,  0.9481821478f,  0.9508633281f,  0.9534768746f,  0.9560223951f,  0.9584995078f,  
	0.960907841f,  0.9632470336f,  0.9655167346f,  0.9677166035f,  0.9698463104f,  0.9719055357f,  0.9738939706f,  0.9758113167f,  0.9776572865f,  0.9794316031f,  
	0.9811340002f,  0.9827642224f,  0.9843220254f,  0.9858071753f,  0.9872194493f,  0.9885586357f,  0.9898245335f,  0.9910169529f,  0.9921357149f,  0.9931806517f, 
	0.9941516066f,  0.9950484339f,  0.9958709992f,  0.9966191789f,  0.9972928609f,  0.997891944f,  0.9984163386f,  0.9988659658f,  0.9992407582f,  0.9995406597f,  
	0.9997656251f,  0.9999156208f,  0.9999906243f,  0.9999906243f,  0.9999156208f,  0.9997656251f,  0.9995406597f,  0.9992407582f,  0.9988659658f,  0.9984163386f,  
	0.997891944f,  0.9972928609f,  0.9966191789f,  0.9958709992f,  0.9950484339f,  0.9941516066f,  0.9931806517f,  0.9921357149f,  0.9910169529f,  0.9898245335f,  
	0.9885586357f,  0.9872194493f,  0.9858071753f,  0.9843220254f,  0.9827642224f,  0.9811340002f,  0.9794316031f,  0.9776572865f,  0.9758113167f,  0.9738939706f, 
	0.9719055357f,  0.9698463104f,  0.9677166035f,  0.9655167346f,  0.9632470336f,  0.960907841f,  0.9584995078f,  0.9560223951f,  0.9534768746f,  0.9508633281f, 
	0.9481821478f,  0.9454337357f,  0.9426185042f,  0.9397368756f,  0.9367892822f,  0.9337761661f,  0.9306979793f,  0.9275551836f,  0.9243482505f,  0.921077661f,  
	0.9177439057f,  0.9143474847f,  0.9108889076f,  0.9073686931f,  0.9037873693f,  0.9001454735f,  0.8964435519f,  0.89268216f,  0.8888618618f,  0.8849832306f,  
	0.8810468481f,  0.8770533049f,  0.8730032f,  0.8688971409f,  0.8647357437f,  0.8605196326f,  0.8562494399f,  0.8519258064f,  0.8475493806f,  0.8431208189f,  
	0.8386407858f,  0.8341099533f,  0.8295290009f,  0.8248986161f,  0.8202194932f,  0.8154923343f,  0.8107178484f,  0.8058967519f,  
	0.8010297678f,  0.7961176263f,  0.7911610643f,  0.7861608253f,  0.7811176593f,  0.776032323f,  0.7709055791f,  0.7657381967f,  0.760530951f, 
	0.7552846231f,  0.75f,  0.7446778745f,  0.7393190448f,  0.733924315f,  0.7284944943f,  0.7230303971f,  0.7175328432f,  0.7120026573f,  0.7064406689f,  
	0.7008477123f,  0.6952246267f,  0.6895722554f,  0.6838914464f,  0.678183052f,  0.6724479283f,  0.6666869357f,  0.6609009385f,  0.6550908046f,  0.6492574055f,  
	0.6434016164f,  0.6375243156f,  0.6316263848f,  0.6257087088f,  0.6197721752f,  0.6138176746f,  0.6078461003f,  0.601858348f,  0.595855316f,  0.5898379047f,  
	0.5838070168f,  0.5777635571f,  0.5717084321f,  0.5656425501f,  0.5595668211f,  0.5534821565f,  0.5473894691f,  0.5412896727f,  0.5351836825f,  0.5290724145f,  
	0.5229567852f,  0.5168377122f,  0.5107161134f,  0.5045929071f,  0.4984690118f,  0.4923453462f,  0.4862228288f,  0.4801023782f,  0.4739849123f,  0.467871349f, 
	0.4617626054f,  0.4556595977f,  0.4495632414f,  0.4434744512f,  0.4373941404f,  0.4313232211f,  0.425262604f,  0.4192131982f,  0.4131759112f,  0.4071516486f,  
	0.4011413143f,  0.3951458097f,  0.3891660342f,  0.383202885f,  0.3772572564f,  0.3713300405f,  0.3654221263f,  0.3595344001f,  0.3536677451f,  0.3478230414f,  
	0.3420011658f,  0.3362029915f,  0.3304293883f,  0.3246812224f,  0.318959356f,  0.3132646475f,  0.3075979512f,  0.301960117f,  0.2963519907f,  0.2907744136f,  
	0.2852282225f,  0.2797142491f,  0.2742333208f,  0.2687862598f,  0.263373883f,  0.2579970025f,  0.2526564247f,  0.247352951f,  0.2420873767f,  0.2368604919f, 
	0.2316730806f,  0.2265259209f,  0.2214197851f,  0.216355439f,  0.2113336423f,  0.2063551484f,  0.2014207041f,  0.1965310497f,  0.1916869185f,  0.1868890374f, 
	0.1821381259f,  0.1774348968f,  0.1727800557f,  0.1681743007f,  0.1636183228f,  0.1591128055f,  0.1546584246f,  0.1502558483f,  0.1459057371f,  0.1416087434f,  
	0.137365512f,  0.1331766792f,  0.1290428736f,  0.1249647151f,  0.1209428156f,  0.1169777784f,  0.1130701983f,  0.1092206614f,  0.1054297453f,  0.1016980186f,  
	0.098026041f,  0.09441436347f,  0.09086352775f,  0.08737406651f,  0.08394650319f,  0.08058135196f,  0.07727911764f,  0.07404029558f,  0.07086537164f,  
	0.0677548221f,  0.06470911356f,  0.06172870291f,  0.05881403724f,  0.05596555379f,  0.05318367984f,  0.05046883271f,  0.04782141965f,  0.0452418378f,  
	0.04273047413f,  0.04028770535f,  0.03791389792f,  0.03560940792f,  0.03337458106f,  0.03120975257f,  0.0291152472f,  0.02709137915f,  0.02513845202f,  
	0.02325675877f,  0.02144658166f,  0.01970819225f,  0.01804185131f,  0.0164478088f,  0.01492630386f,  0.01347756471f,  0.01210180869f,  0.01079924216f, 
	0.009570060539f,  0.008414448203f,  0.007332578507f,  0.006324613744f,  0.005390705117f,  0.004530992723f,  0.003745605525f,  0.003034661341f,  0.002398266818f,  
	0.001836517423f,  0.001349497423f,  0.0009372798749f,  0.0005999266166f,  0.000337488254f,  0.0001500041554f,  3.750244528e-005f
};
static const float syn_hwin1024[]= {
	9.394002138e-006f,  3.757565556e-005f,  8.454390132e-005f,  0.0001502969745f,  0.0002348324045f,  0.0003381470146f,  0.0004602369228f,  0.0006010975414f,  0.0007607235774f,  0.0009391090328f,  0.001136247204f,  0.001352130685f,  0.001586751362f,  0.001840100419f,  
	0.002112168337f,  0.002402944893f,  0.002712419159f,  0.003040579508f,  0.003387413609f,  0.003752908428f,  0.004137050232f,  0.004539824587f,  0.004961216357f,  0.005401209709f,  0.00585978811f,  0.006336934327f,  0.006832630432f,  0.007346857798f, 
	0.007879597103f,  0.008430828328f,  0.009000530761f,  0.009588682994f,  0.01019526293f,  0.01082024777f,  0.01146361403f,  0.01212533754f,  0.01280539343f,  0.01350375615f,  0.01422039946f,  0.01495529643f,  0.01570841944f,  0.01647974019f,  0.01726922971f, 
	0.01807685832f,  0.01890259568f,  0.01974641076f,  0.02060827185f,  0.02148814657f,  0.02238600186f,  0.02330180397f,  0.0242355185f,  0.02518711036f,  0.02615654379f,  0.02714378237f,  0.028148789f,  0.02917152591f,  0.03021195468f,  0.03127003621f,  
	0.03234573074f,  0.03343899785f,  0.03454979646f,  0.03567808483f,  0.03682382056f,  0.03798696061f,  0.03916746126f,  0.04036527816f,  0.0415803663f,  0.04281268002f,  0.04406217301f,  0.04532879833f,  0.04661250837f,  0.04791325491f,  0.04923098906f,  
	0.05056566132f,  0.05191722152f,  0.05328561888f,  0.05467080199f,  0.05607271879f,  0.05749131661f,  0.05892654214f,  0.06037834144f,  0.06184665998f,  0.06333144257f,  0.06483263342f,  0.06635017612f,  0.06788401365f,  0.06943408837f,  0.07100034205f,  
	0.07258271582f,  0.07418115022f,  0.0757955852f,  0.07742596008f,  0.07907221361f,  0.08073428393f,  0.08241210858f,  0.08410562452f,  0.0858147681f,  0.08753947512f,  0.08927968075f,  0.09103531962f,  0.09280632574f,  0.09459263258f,  0.09639417301f,  
	0.09821087933f,  0.1000426833f,  0.101889516f,  0.1037513082f,  0.1056279898f,  0.1075194903f,  0.1094257387f,  0.1113466633f,  0.113282192f,  0.1152322519f,  0.1171967699f,  0.1191756721f,  0.1211688842f,  0.1231763312f,  0.1251979378f,  0.1272336279f,  
	0.1292833251f,  0.1313469523f,  0.1334244321f,  0.1355156863f,  0.1376206364f,  0.1397392032f,  0.1418713072f,  0.1440168683f,  0.1461758057f,  0.1483480385f,  0.150533485f,  0.152732063f,  0.1549436899f,  0.1571682827f,  0.1594057577f,  0.161656031f,  
	0.1639190178f,  0.1661946332f,  0.1684827917f,  0.1707834073f,  0.1730963935f,  0.1754216634f,  0.1777591297f,  0.1801087046f,  0.1824702997f,  0.1848438262f,  0.1872291951f,  0.1896263167f,  0.1920351009f,  0.1944554571f,  0.1968872945f,  0.1993305217f,  
	0.2017850468f,  0.2042507777f,  0.2067276216f,  0.2092154855f,  0.211714276f,  0.2142238991f,  0.2167442605f,  0.2192752655f,  0.2218168191f,  0.2243688256f,  0.2269311893f,  0.2295038138f,  0.2320866025f,  0.2346794583f,  0.2372822838f,  0.2398949812f,  
	0.2425174522f,  0.2451495985f,  0.247791321f,  0.2504425205f,  0.2531030974f,  0.2557729516f,  0.258451983f,  0.2611400907f,  0.2638371739f,  0.2665431311f,  0.2692578607f,  0.2719812607f,  0.2747132287f,  0.2774536621f,  0.2802024578f,  0.2829595127f,  
	0.2857247232f,  0.2884979852f,  0.2912791946f,  0.2940682469f,  0.2968650373f,  0.2996694607f,  0.3024814118f,  0.3053007848f,  0.3081274738f,  0.3109613726f,  0.3138023747f,  0.3166503734f,  0.3195052617f,  0.3223669322f,  0.3252352774f,  0.3281101896f,  
	0.3309915608f,  0.3338792826f,  0.3367732466f,  0.339673344f,  0.3425794658f,  0.3454915028f,  0.3484093457f,  0.3513328847f,  0.3542620101f,  0.3571966117f,  0.3601365792f,  0.3630818023f,  0.3660321703f,  0.3689875722f,  0.371947897f,  0.3749130335f,  
	0.3778828703f,  0.3808572958f,  0.3838361981f,  0.3868194655f,  0.3898069857f,  0.3927986465f,  0.3957943355f,  0.3987939402f,  0.4017973478f,  0.4048044454f,  0.4078151201f,  0.4108292588f,  0.4138467481f,  0.4168674747f,  0.4198913252f,  0.4229181857f,  
	0.4259479427f,  0.4289804823f,  0.4320156905f,  0.4350534533f,  0.4380936566f,  0.441136186f,  0.4441809273f,  0.447227766f,  0.4502765877f,  0.4533272779f,  0.4563797218f,  0.4594338047f,  0.462489412f,  0.4655464288f,  0.4686047402f,  0.4716642314f,  
	0.4747247872f,  0.4777862928f,  0.4808486332f,  0.4839116931f,  0.4869753576f,  0.4900395115f,  0.4931040396f,  0.4961688269f,  0.4992337582f,  0.5022987182f,  0.5053635919f,  0.508428264f,  0.5114926195f,  0.514556543f,  0.5176199196f,  0.5206826341f,  
	0.5237445715f,  0.5268056166f,  0.5298656545f,  0.5329245701f,  0.5359822486f,  0.539038575f,  0.5420934345f,  0.5451467122f,  0.5481982936f,  0.5512480638f,  0.5542959084f,  0.5573417127f,  0.5603853624f,  0.563426743f,  0.5664657402f,  0.56950224f,  
	0.5725361282f,  0.5755672907f,  0.5785956137f,  0.5816209834f,  0.5846432861f,  0.5876624083f,  0.5906782365f,  0.5936906573f,  0.5966995576f,  0.5997048243f,  0.6027063445f,  0.6057040055f,  0.6086976945f,  0.611687299f,  0.6146727068f,  0.6176538057f,  
	0.6206304836f,  0.6236026287f,  0.6265701292f,  0.6295328738f,  0.6324907511f,  0.6354436499f,  0.6383914592f,  0.6413340684f,  0.6442713668f,  0.647203244f,  0.65012959f,  0.6530502946f,  0.6559652483f,  0.6588743414f,  0.6617774646f,  0.6646745089f, 
	0.6675653654f,  0.6704499254f,  0.6733280806f,  0.6761997228f,  0.6790647442f,  0.681923037f,  0.6847744938f,  0.6876190076f,  0.6904564714f,  0.6932867786f,  0.6961098229f,  0.6989254981f,  0.7017336985f,  0.7045343186f,  0.7073272531f,  0.710112397f,  
	0.7128896458f,  0.715658895f,  0.7184200406f,  0.7211729789f,  0.7239176064f,  0.7266538199f,  0.7293815167f,  0.7321005943f,  0.7348109504f,  0.7375124833f,  0.7402050914f,  0.7428886735f,  0.7455631289f,  0.748228357f,  0.7508842577f,  0.7535307311f,  
	0.7561676779f,  0.7587949989f,  0.7614125954f,  0.7640203691f,  0.7666182219f,  0.7692060563f,  0.771783775f,  0.7743512812f,  0.7769084783f,  0.7794552703f,  0.7819915615f,  0.7845172566f,  0.7870322606f,  0.7895364792f,  0.7920298181f,  0.7945121837f,  
	0.7969834827f,  0.7994436222f,  0.8018925099f,  0.8043300536f,  0.8067561619f,  0.8091707434f,  0.8115737076f,  0.813964964f,  0.8163444229f,  0.8187119949f,  0.8210675909f,  0.8234111225f,  0.8257425016f,  0.8280616405f,  0.8303684523f,  0.8326628501f,  
	0.8349447477f,  0.8372140595f,  0.8394707001f,  0.8417145848f,  0.8439456292f,  0.8461637495f,  0.8483688623f,  0.8505608849f,  0.8527397347f,  0.85490533f,  0.8570575894f,  0.8591964319f,  0.8613217774f,  0.8634335457f,  0.8655316577f,  0.8676160345f,  
	0.8696865977f,  0.8717432696f,  0.8737859729f,  0.8758146307f,  0.877829167f,  0.8798295059f,  0.8818155724f,  0.8837872917f,  0.8857445899f,  0.8876873933f,  0.889615629f,  0.8915292245f,  0.8934281079f,  0.8953122078f,  0.8971814534f,  0.8990357746f,  
	0.9008751016f,  0.9026993653f,  0.9045084972f,  0.9063024292f,  0.9080810941f,  0.9098444249f,  0.9115923553f,  0.9133248198f,  0.9150417531f,  0.9167430909f,  0.9184287691f,  0.9200987244f,  0.9217528941f,  0.923391216f,  0.9250136286f,  0.9266200708f, 
	0.9282104824f,  0.9297848035f,  0.931342975f,  0.9328849384f,  0.9344106357f,  0.9359200095f,  0.9374130033f,  0.9388895608f,  0.9403496265f,  0.9417931457f,  0.9432200641f,  0.944630328f,  0.9460238845f,  0.9474006812f,  0.9487606664f,  0.950103789f,  
	0.9514299984f,  0.9527392449f,  0.9540314792f,  0.9553066529f,  0.956564718f,  0.9578056272f,  0.9590293338f,  0.960235792f,  0.9614249564f,  0.9625967822f,  0.9637512256f,  0.9648882429f,  0.9660077917f,  0.9671098297f,  0.9681943156f,  0.9692612087f,  
	0.9703104688f,  0.9713420564f,  0.972355933f,  0.9733520603f,  0.9743304009f,  0.975290918f,  0.9762335756f,  0.9771583383f,  0.9780651713f,  0.9789540404f,  0.9798249124f,  0.9806777545f,  0.9815125347f,  0.9823292215f,  0.9831277843f,  0.9839081931f,  
	0.9846704186f,  0.9854144321f,  0.9861402056f,  0.9868477119f,  0.9875369245f,  0.9882078173f,  0.9888603653f,  0.9894945438f,  0.9901103291f,  0.990707698f,  0.991286628f,  0.9918470975f,  0.9923890853f,  0.9929125711f,  0.9934175352f,  0.9939039587f,  
	0.9943718232f,  0.9948211111f,  0.9952518057f,  0.9956638907f,  0.9960573507f,  0.9964321707f,  0.9967883369f,  0.9971258357f,  0.9974446545f,  0.9977447813f,  0.9980262049f,  0.9982889146f,  0.9985329006f,  0.9987581537f,  0.9989646655f,  0.9991524282f,  
	0.9993214348f,  0.9994716788f,  0.9996031547f,  0.9997158575f,  0.9998097829f,  0.9998849275f,  0.9999412885f,  0.9999788636f,  0.9999976515f,  0.9999976515f,  0.9999788636f,  0.9999412885f,  0.9998849275f,  0.9998097829f,  0.9997158575f,  0.9996031547f, 
	0.9994716788f,  0.9993214348f,  0.9991524282f,  0.9989646655f,  0.9987581537f,  0.9985329006f,  0.9982889146f,  0.9980262049f,  0.9977447813f,  0.9974446545f,  0.9971258357f,  0.9967883369f,  0.9964321707f,  0.9960573507f,  0.9956638907f,  0.9952518057f, 
	0.9948211111f,  0.9943718232f,  0.9939039587f,  0.9934175352f,  0.9929125711f,  0.9923890853f,  0.9918470975f,  0.991286628f,  0.990707698f,  0.9901103291f,  0.9894945438f,  0.9888603653f,  0.9882078173f,  0.9875369245f,  0.9868477119f,  0.9861402056f,  
	0.9854144321f,  0.9846704186f,  0.9839081931f,  0.9831277843f,  0.9823292215f,  0.9815125347f,  0.9806777545f,  0.9798249124f,  0.9789540404f,  0.9780651713f,  0.9771583383f,  0.9762335756f,  0.975290918f,  0.9743304009f,  0.9733520603f,  0.972355933f,  
	0.9713420564f,  0.9703104688f,  0.9692612087f,  0.9681943156f,  0.9671098297f,  0.9660077917f,  0.9648882429f,  0.9637512256f,  0.9625967822f,  0.9614249564f,  0.960235792f,  0.9590293338f,  0.9578056272f,  0.956564718f,  0.9553066529f,  0.9540314792f,  
	0.9527392449f,  0.9514299984f,  0.950103789f,  0.9487606664f,  0.9474006812f,  0.9460238845f,  0.944630328f,  0.9432200641f,  0.9417931457f,  0.9403496265f,  0.9388895608f,  0.9374130033f,  0.9359200095f,  0.9344106357f,  0.9328849384f,  0.931342975f,  
	0.9297848035f,  0.9282104824f,  0.9266200708f,  0.9250136286f,  0.923391216f,  0.9217528941f,  0.9200987244f,  0.9184287691f,  0.9167430909f,  0.9150417531f,  0.9133248198f,  0.9115923553f,  0.9098444249f,  0.9080810941f,  0.9063024292f,  0.9045084972f,  
	0.9026993653f,  0.9008751016f,  0.8990357746f,  0.8971814534f,  0.8953122078f,  0.8934281079f,  0.8915292245f,  0.889615629f,  0.8876873933f,  0.8857445899f,  0.8837872917f,  0.8818155724f,  0.8798295059f,  0.877829167f,  0.8758146307f,  0.8737859729f, 
	0.8717432696f,  0.8696865977f,  0.8676160345f,  0.8655316577f,  0.8634335457f,  0.8613217774f,  0.8591964319f,  0.8570575894f,  0.85490533f,  0.8527397347f,  0.8505608849f,  0.8483688623f,  0.8461637495f,  0.8439456292f,  0.8417145848f,  0.8394707001f, 
	0.8372140595f,  0.8349447477f,  0.8326628501f,  0.8303684523f,  0.8280616405f,  0.8257425016f,  0.8234111225f,  0.8210675909f,  0.8187119949f,  0.8163444229f,  0.813964964f,  0.8115737076f,  0.8091707434f,  0.8067561619f,  0.8043300536f,  0.8018925099f, 
	0.7994436222f,  0.7969834827f,  0.7945121837f,  0.7920298181f,  0.7895364792f,  0.7870322606f,  0.7845172566f,  0.7819915615f,  0.7794552703f,  0.7769084783f,  0.7743512812f,  0.771783775f,  0.7692060563f,  0.7666182219f,  0.7640203691f,  0.7614125954f, 
	0.7587949989f,  0.7561676779f,  0.7535307311f,  0.7508842577f,  0.748228357f,  0.7455631289f,  0.7428886735f,  0.7402050914f,  0.7375124833f,  0.7348109504f,  0.7321005943f,  0.7293815167f,  0.7266538199f,  0.7239176064f,  0.7211729789f,  0.7184200406f,  
	0.715658895f,  0.7128896458f,  0.710112397f,  0.7073272531f,  0.7045343186f,  0.7017336985f,  0.6989254981f,  0.6961098229f,  0.6932867786f,  0.6904564714f,  0.6876190076f,  0.6847744938f,  0.681923037f,  0.6790647442f,  0.6761997228f,  0.6733280806f,  
	0.6704499254f,  0.6675653654f,  0.6646745089f,  0.6617774646f,  0.6588743414f,  0.6559652483f,  0.6530502946f,  0.65012959f,  0.647203244f,  0.6442713668f,  0.6413340684f,  0.6383914592f,  0.6354436499f,  0.6324907511f,  0.6295328738f,  0.6265701292f,  
	0.6236026287f,  0.6206304836f,  0.6176538057f,  0.6146727068f,  0.611687299f,  0.6086976945f,  0.6057040055f,  0.6027063445f,  0.5997048243f,  0.5966995576f,  0.5936906573f,  0.5906782365f,  0.5876624083f,  0.5846432861f,  0.5816209834f,  0.5785956137f,  
	0.5755672907f,  0.5725361282f,  0.56950224f,  0.5664657402f,  0.563426743f,  0.5603853624f,  0.5573417127f,  0.5542959084f,  0.5512480638f,  0.5481982936f,  0.5451467122f,  0.5420934345f,  0.539038575f,  0.5359822486f,  0.5329245701f,  0.5298656545f,  
	0.5268056166f,  0.5237445715f,  0.5206826341f,  0.5176199196f,  0.514556543f,  0.5114926195f,  0.508428264f,  0.5053635919f,  0.5022987182f,  0.4992337582f,  0.4961688269f,  0.4931040396f,  0.4900395115f,  0.4869753576f,  0.4839116931f,  0.4808486332f,  
	0.4777862928f,  0.4747247872f,  0.4716642314f,  0.4686047402f,  0.4655464288f,  0.462489412f,  0.4594338047f,  0.4563797218f,  0.4533272779f,  0.4502765877f,  0.447227766f,  0.4441809273f,  0.441136186f,  0.4380936566f,  0.4350534533f,  0.4320156905f,  
	0.4289804823f,  0.4259479427f,  0.4229181857f,  0.4198913252f,  0.4168674747f,  0.4138467481f,  0.4108292588f,  0.4078151201f,  0.4048044454f,  0.4017973478f,  0.3987939402f,  0.3957943355f,  0.3927986465f,  0.3898069857f,  0.3868194655f,  0.3838361981f,  
	0.3808572958f,  0.3778828703f,  0.3749130335f,  0.371947897f,  0.3689875722f,  0.3660321703f,  0.3630818023f,  0.3601365792f,  0.3571966117f,  0.3542620101f,  0.3513328847f,  0.3484093457f,  0.3454915028f,  0.3425794658f,  0.339673344f,  0.3367732466f,  
	0.3338792826f,  0.3309915608f,  0.3281101896f,  0.3252352774f,  0.3223669322f,  0.3195052617f,  0.3166503734f,  0.3138023747f,  0.3109613726f,  0.3081274738f,  0.3053007848f,  0.3024814118f,  0.2996694607f,  0.2968650373f,  0.2940682469f,  0.2912791946f,  
	0.2884979852f,  0.2857247232f,  0.2829595127f,  0.2802024578f,  0.2774536621f,  0.2747132287f,  0.2719812607f,  0.2692578607f,  0.2665431311f,  0.2638371739f,  0.2611400907f,  0.258451983f,  0.2557729516f,  0.2531030974f,  0.2504425205f,  0.247791321f,  
	0.2451495985f,  0.2425174522f,  0.2398949812f,  0.2372822838f,  0.2346794583f,  0.2320866025f,  0.2295038138f,  0.2269311893f,  0.2243688256f,  0.2218168191f,  0.2192752655f,  0.2167442605f,  0.2142238991f,  0.211714276f,  0.2092154855f,  0.2067276216f,  
	0.2042507777f,  0.2017850468f,  0.1993305217f,  0.1968872945f,  0.1944554571f,  0.1920351009f,  0.1896263167f,  0.1872291951f,  0.1848438262f,  0.1824702997f,  0.1801087046f,  0.1777591297f,  0.1754216634f,  0.1730963935f,  0.1707834073f,  0.1684827917f,  
	0.1661946332f,  0.1639190178f,  0.161656031f,  0.1594057577f,  0.1571682827f,  0.1549436899f,  0.152732063f,  0.150533485f,  0.1483480385f,  0.1461758057f,  0.1440168683f,  0.1418713072f,  0.1397392032f,  0.1376206364f,  0.1355156863f,  0.1334244321f,  
	0.1313469523f,  0.1292833251f,  0.1272336279f,  0.1251979378f,  0.1231763312f,  0.1211688842f,  0.1191756721f,  0.1171967699f,  0.1152322519f,  0.113282192f,  0.1113466633f,  0.1094257387f,  0.1075194903f,  0.1056279898f,  0.1037513082f,  0.101889516f,  
	0.1000426833f,  0.09821087933f,  0.09639417301f,  0.09459263258f,  0.09280632574f,  0.09103531962f,  0.08927968075f,  0.08753947512f,  0.0858147681f,  0.08410562452f,  0.08241210858f,  0.08073428393f,  0.07907221361f,  0.07742596008f,  0.0757955852f,  
	0.07418115022f,  0.07258271582f,  0.07100034205f,  0.06943408837f,  0.06788401365f,  0.06635017612f,  0.06483263342f,  0.06333144257f,  0.06184665998f,  0.06037834144f,  0.05892654214f,  0.05749131661f,  0.05607271879f,  0.05467080199f,  0.05328561888f,  
	0.05191722152f,  0.05056566132f,  0.04923098906f,  0.04791325491f,  0.04661250837f,  0.04532879833f,  0.04406217301f,  0.04281268002f,  0.0415803663f,  0.04036527816f,  0.03916746126f,  0.03798696061f,  0.03682382056f,  0.03567808483f,  0.03454979646f,  
	0.03343899785f,  0.03234573074f,  0.03127003621f,  0.03021195468f,  0.02917152591f,  0.028148789f,  0.02714378237f,  0.02615654379f,  0.02518711036f,  0.0242355185f,  0.02330180397f,  0.02238600186f,  0.02148814657f,  0.02060827185f,  0.01974641076f,  
	0.01890259568f,  0.01807685832f,  0.01726922971f,  0.01647974019f,  0.01570841944f,  0.01495529643f,  0.01422039946f,  0.01350375615f,  0.01280539343f,  0.01212533754f,  0.01146361403f,  0.01082024777f,  0.01019526293f,  0.009588682994f,  0.009000530761f,  
	0.008430828328f,  0.007879597103f,  0.007346857798f,  0.006832630432f,  0.006336934327f,  0.00585978811f,  0.005401209709f,  0.004961216357f,  0.004539824587f,  0.004137050232f,  0.003752908428f,  0.003387413609f,  0.003040579508f,  0.002712419159f,  
	0.002402944893f,  0.002112168337f,  0.001840100419f,  0.001586751362f,  0.001352130685f,  0.001136247204f,  0.0009391090328f,  0.0007607235774f,  0.0006010975414f,  0.0004602369228f,  0.0003381470146f,  0.0002348324045f,  0.0001502969745f,  8.454390132e-005f, 
	3.757565556e-005f,  9.394002138e-006f
};
class F2Ttransformer
{
public:
	F2Ttransformer(void);
	~F2Ttransformer(void);
	
	/***************************************************
	name:    InitFDanaly
	para:    size  (IN)
	content: initial parameter
	***************************************************/
	void InitFDanaly(const int size);
	
	/***************************************************
	name:    F2T
	para:    inbuf   (IN)  
	         outbuf  (OUT)
	content: the fourier inverse transform
	***************************************************/
	void F2T(const float *inbuf, float *outbuf);

private:
	float* AnaWin;
	
	/***************************************************
	name:    UpdateFDbuffer
	para:    data   (IN)  
	content: update data
	***************************************************/
	inline	void UpdateFDbuffer( float* data);

	/***************************************************
	name:    FreeFDanaly
	content: release the allocated memory
	***************************************************/
	void FreeFDanaly();

	int m_shift;         //the length of the offset
	int m_fft_len;	     //the length of data
	float* m_input_tim;	 //store the original data
	float* m_input_spe;  //temporary variable
	float* m_output_spe; //temporary variable
};
#endif

