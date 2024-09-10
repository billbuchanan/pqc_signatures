#include "params.h"

#include "domain.h"

// preon_128
const Parameters preon = {
    .aes_size = 128,
    .field_bitsize = 192,
    .field_bytesize = 24,
    .field_words = 3,
    .hash_bitsize = 384,
    .hash_bytesize = 48,
    .hash_zk_bytesize = 96,
    .verifier_messages_count = {4, 19, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    .query_bound = 763,
    .num_interaction_rounds = 15,
    .input_variable_domain = {
    .size = 64,
    .basis_len = 6,
    .basis = (uint64_t []){0ull, 0ull, 1ull, 0ull, 0ull, 2ull, 0ull, 0ull, 4ull, 0ull, 0ull, 8ull, 0ull, 0ull, 16ull, 0ull, 0ull, 32ull},
    .shift = (uint64_t []){0ull, 0ull, 0ull}},
    .variable_domain = {
    .size = 4096,
    .basis_len = 12,
    .basis = (uint64_t []){0ull, 0ull, 1ull, 0ull, 0ull, 2ull, 0ull, 0ull, 4ull, 0ull, 0ull, 8ull, 0ull, 0ull, 16ull, 0ull, 0ull, 32ull, 0ull, 0ull, 64ull, 0ull, 0ull, 128ull, 0ull, 0ull, 256ull, 0ull, 0ull, 512ull, 0ull, 0ull, 1024ull, 0ull, 0ull, 2048ull},
    .shift = (uint64_t []){0ull, 0ull, 0ull}},
    .constraint_domain = {
    .size = 4096,
    .basis_len = 12,
    .basis = (uint64_t []){0ull, 0ull, 1ull, 0ull, 0ull, 2ull, 0ull, 0ull, 4ull, 0ull, 0ull, 8ull, 0ull, 0ull, 16ull, 0ull, 0ull, 32ull, 0ull, 0ull, 64ull, 0ull, 0ull, 128ull, 0ull, 0ull, 256ull, 0ull, 0ull, 512ull, 0ull, 0ull, 1024ull, 0ull, 0ull, 2048ull},
    .shift = (uint64_t []){0ull, 0ull, 0ull}},
    .summation_domain = {
    .size = 4096,
    .basis_len = 12,
    .basis = (uint64_t []){0ull, 0ull, 1ull, 0ull, 0ull, 2ull, 0ull, 0ull, 4ull, 0ull, 0ull, 8ull, 0ull, 0ull, 16ull, 0ull, 0ull, 32ull, 0ull, 0ull, 64ull, 0ull, 0ull, 128ull, 0ull, 0ull, 256ull, 0ull, 0ull, 512ull, 0ull, 0ull, 1024ull, 0ull, 0ull, 2048ull},
    .shift = (uint64_t []){0ull, 0ull, 0ull}},
    .codeword_domain = {
    .size = 524288,
    .basis_len = 19,
    .basis = (uint64_t []){0ull, 0ull, 1ull, 0ull, 0ull, 2ull, 0ull, 0ull, 4ull, 0ull, 0ull, 8ull, 0ull, 0ull, 16ull, 0ull, 0ull, 32ull, 0ull, 0ull, 64ull, 0ull, 0ull, 128ull, 0ull, 0ull, 256ull, 0ull, 0ull, 512ull, 0ull, 0ull, 1024ull, 0ull, 0ull, 2048ull, 0ull, 0ull, 4096ull, 0ull, 0ull, 8192ull, 0ull, 0ull, 16384ull, 0ull, 0ull, 32768ull, 0ull, 0ull, 65536ull, 0ull, 0ull, 131072ull, 0ull, 0ull, 262144ull},
    .shift = (uint64_t []){0ull, 0ull, 524288ull}},
    .fri_domains = {{
    .size = 524288,
    .basis_len = 19,
    .basis = (uint64_t []){0ull, 0ull, 1ull, 0ull, 0ull, 2ull, 0ull, 0ull, 4ull, 0ull, 0ull, 8ull, 0ull, 0ull, 16ull, 0ull, 0ull, 32ull, 0ull, 0ull, 64ull, 0ull, 0ull, 128ull, 0ull, 0ull, 256ull, 0ull, 0ull, 512ull, 0ull, 0ull, 1024ull, 0ull, 0ull, 2048ull, 0ull, 0ull, 4096ull, 0ull, 0ull, 8192ull, 0ull, 0ull, 16384ull, 0ull, 0ull, 32768ull, 0ull, 0ull, 65536ull, 0ull, 0ull, 131072ull, 0ull, 0ull, 262144ull},
    .shift = (uint64_t []){0ull, 0ull, 524288ull}},
{
    .size = 262144,
    .basis_len = 18,
    .basis = (uint64_t []){0ull, 0ull, 6ull, 0ull, 0ull, 20ull, 0ull, 0ull, 72ull, 0ull, 0ull, 272ull, 0ull, 0ull, 1056ull, 0ull, 0ull, 4160ull, 0ull, 0ull, 16512ull, 0ull, 0ull, 65792ull, 0ull, 0ull, 262656ull, 0ull, 0ull, 1049600ull, 0ull, 0ull, 4196352ull, 0ull, 0ull, 16781312ull, 0ull, 0ull, 67117056ull, 0ull, 0ull, 268451840ull, 0ull, 0ull, 1073774592ull, 0ull, 0ull, 4295032832ull, 0ull, 0ull, 17180000256ull, 0ull, 0ull, 68719738880ull},
    .shift = (uint64_t []){0ull, 0ull, 274878431232ull}},
{
    .size = 131072,
    .basis_len = 17,
    .basis = (uint64_t []){0ull, 0ull, 360ull, 0ull, 0ull, 4592ull, 0ull, 0ull, 67424ull, 0ull, 0ull, 1055936ull, 0ull, 0ull, 16806272ull, 0ull, 0ull, 268550912ull, 0ull, 0ull, 4295427584ull, 0ull, 0ull, 68721314816ull, 0ull, 0ull, 1099518973952ull, 0ull, 0ull, 17592215416832ull, 0ull, 0ull, 281475094175744ull, 0ull, 0ull, 4503600097181696ull, 0ull, 0ull, 72057595917074432ull, 0ull, 0ull, 1152921512123236352ull, 0ull, 0ull, 30065164315ull, 0ull, 0ull, 120259871152ull, 0ull, 0ull, 481037916928ull},
    .shift = (uint64_t []){0ull, 0ull, 1924148604928ull}},
{
    .size = 65536,
    .basis_len = 16,
    .basis = (uint64_t []){0ull, 0ull, 18245760ull, 0ull, 0ull, 4319836928ull, 0ull, 0ull, 1099910667776ull, 0ull, 0ull, 281481375194112ull, 0ull, 0ull, 72057696337049600ull, 0ull, 0ull, 1636483952667ull, 0ull, 0ull, 26182527417088ull, 0ull, 0ull, 418915553820672ull, 0ull, 0ull, 6702629731926016ull, 0ull, 0ull, 107242090889936896ull, 0ull, 0ull, 1715901248742227968ull, 0ull, 0ull, 9014799494681460763ull, 0ull, 0ull, 14627693251914105329ull, 0ull, 0ull, 6648734162682ull, 0ull, 0ull, 26594687106800ull, 0ull, 0ull, 106378229890816ull},
    .shift = (uint64_t []){0ull, 0ull, 425517433745408ull}},
{
    .size = 32768,
    .basis_len = 15,
    .basis = (uint64_t []){0ull, 0ull, 78274681211224091ull, 0ull, 0ull, 1687463513465815067ull, 0ull, 0ull, 8988296383555312369ull, 0ull, 0ull, 14393933788395009471ull, 0ull, 0ull, 7906007890015401141ull, 0ull, 0ull, 15781322294011423466ull, 0ull, 0ull, 12813109461703663119ull, 0ull, 0ull, 1994160189539999749ull, 0ull, 0ull, 12275766169714957820ull, 0ull, 0ull, 9318856278138094875ull, 0ull, 0ull, 176229507072594925ull, 0ull, 0ull, 7554707376080922675ull, 0ull, 0ull, 8758347017533341550ull, 0ull, 0ull, 16590670462996896627ull, 0ull, 0ull, 11012212934298695137ull},
    .shift = (uint64_t []){0ull, 0ull, 8233531721804355497ull}},
{
    .size = 16384,
    .basis_len = 14,
    .basis = (uint64_t []){0ull, 0ull, 8244804748658156366ull, 0ull, 0ull, 6964524709014517257ull, 0ull, 0ull, 578243005985270896ull, 0ull, 0ull, 17277335748864455019ull, 0ull, 0ull, 18053493123004363615ull, 0ull, 0ull, 10607133894588926735ull, 0ull, 0ull, 17771630945875104135ull, 0ull, 0ull, 266999502398134886ull, 0ull, 0ull, 7901597046524247948ull, 0ull, 0ull, 599988404761654045ull, 0ull, 0ull, 9924105546285818945ull, 0ull, 0ull, 18357335082377083378ull, 0ull, 0ull, 5628605430546204695ull, 0ull, 0ull, 7764037522679191414ull},
    .shift = (uint64_t []){0ull, 0ull, 14483935372246150029ull}},
{
    .size = 8192,
    .basis_len = 13,
    .basis = (uint64_t []){0ull, 0ull, 8749050644146709186ull, 0ull, 0ull, 3762997275846411469ull, 0ull, 0ull, 11907229328435556158ull, 0ull, 0ull, 4555794481886719399ull, 0ull, 0ull, 7622730220496178298ull, 0ull, 0ull, 14880951363819466616ull, 0ull, 0ull, 8278445872699611729ull, 0ull, 0ull, 5443067111753567332ull, 0ull, 0ull, 6564153150106746978ull, 0ull, 0ull, 435222840030641386ull, 0ull, 0ull, 10382376087310256901ull, 0ull, 0ull, 10085380629942975569ull, 0ull, 0ull, 17890425171306669005ull},
    .shift = (uint64_t []){0ull, 0ull, 9569079202536107825ull}},
{
    .size = 4096,
    .basis_len = 12,
    .basis = (uint64_t []){0ull, 0ull, 1085473640648473244ull, 0ull, 0ull, 13257388749725124263ull, 0ull, 0ull, 8640965300599897156ull, 0ull, 0ull, 4260877643451432541ull, 0ull, 0ull, 13939281692944205708ull, 0ull, 0ull, 1226547897764449325ull, 0ull, 0ull, 7799250065211332859ull, 0ull, 0ull, 10952656440188270193ull, 0ull, 0ull, 841854861858612556ull, 0ull, 0ull, 475103271713423445ull, 0ull, 0ull, 10045090352156236796ull, 0ull, 0ull, 15910286022844060896ull},
    .shift = (uint64_t []){0ull, 0ull, 11606038052156370149ull}},
{
    .size = 2048,
    .basis_len = 11,
    .basis = (uint64_t []){0ull, 0ull, 7640059558905957125ull, 0ull, 0ull, 15245751680780821412ull, 0ull, 0ull, 143612520775495321ull, 0ull, 0ull, 11567511995096082708ull, 0ull, 0ull, 3961294767110061651ull, 0ull, 0ull, 15102218351729071184ull, 0ull, 0ull, 15120760296663629047ull, 0ull, 0ull, 17293737371726564116ull, 0ull, 0ull, 9032938969889952284ull, 0ull, 0ull, 1654010257632898653ull, 0ull, 0ull, 15948625745157323839ull},
    .shift = (uint64_t []){0ull, 0ull, 16562692581723518366ull}},
{
    .size = 1024,
    .basis_len = 10,
    .basis = (uint64_t []){0ull, 0ull, 15191009085390685913ull, 0ull, 0ull, 10195376274664516890ull, 0ull, 0ull, 16509755171424326081ull, 0ull, 0ull, 16029578439158647856ull, 0ull, 0ull, 12437422469226935224ull, 0ull, 0ull, 4967066667814387433ull, 0ull, 0ull, 3752653742527772082ull, 0ull, 0ull, 15898927856343130875ull, 0ull, 0ull, 10135895550779828266ull, 0ull, 0ull, 15039616053192992243ull},
    .shift = (uint64_t []){0ull, 0ull, 695257828170014123ull}},
{
    .size = 512,
    .basis_len = 9,
    .basis = (uint64_t []){0ull, 0ull, 7878391778441664005ull, 0ull, 0ull, 3025435752272433020ull, 0ull, 0ull, 3577674312705047675ull, 0ull, 0ull, 4943399500596486068ull, 0ull, 0ull, 3688776378962563934ull, 0ull, 0ull, 2098005245367452589ull, 0ull, 0ull, 11395895846190749400ull, 0ull, 0ull, 16251568249814280648ull, 0ull, 0ull, 14801241343801460877ull},
    .shift = (uint64_t []){0ull, 0ull, 2482522441354917599ull}},
{
    .size = 256,
    .basis_len = 8,
    .basis = (uint64_t []){0ull, 0ull, 15027479999479429674ull, 0ull, 0ull, 16383687819482525314ull, 0ull, 0ull, 13559777212733457364ull, 0ull, 0ull, 14978689530819424709ull, 0ull, 0ull, 389518377826715458ull, 0ull, 0ull, 16426555764587873163ull, 0ull, 0ull, 11147965321808188764ull, 0ull, 0ull, 8650774043498647375ull},
    .shift = (uint64_t []){0ull, 0ull, 9565806630786532672ull}},
{
    .size = 128,
    .basis_len = 7,
    .basis = (uint64_t []){0ull, 0ull, 45258276057990961ull, 0ull, 0ull, 18395008089186019805ull, 0ull, 0ull, 7259194857137784930ull, 0ull, 0ull, 1561631287023002294ull, 0ull, 0ull, 12696134591616488608ull, 0ull, 0ull, 7993368573502448038ull, 0ull, 0ull, 10774989307679127350ull},
    .shift = (uint64_t []){0ull, 0ull, 6919237805146469028ull}},
{
    .size = 64,
    .basis_len = 6,
    .basis = (uint64_t []){0ull, 0ull, 4327488829911782419ull, 0ull, 0ull, 12491284962065956391ull, 0ull, 0ull, 13758147461561744591ull, 0ull, 0ull, 2821982089099363728ull, 0ull, 0ull, 1164800802981245030ull, 0ull, 0ull, 16740263891185599165ull},
    .shift = (uint64_t []){0ull, 0ull, 8149678633002775748ull}},
{
    .size = 32,
    .basis_len = 5,
    .basis = (uint64_t []){0ull, 0ull, 436120406283855025ull, 0ull, 0ull, 12529138562996631321ull, 0ull, 0ull, 12626266604873193335ull, 0ull, 0ull, 2854032478056133876ull, 0ull, 0ull, 10317652576331147404ull},
    .shift = (uint64_t []){0ull, 0ull, 1699289732637539126ull}}},
    .fri_localizer_domains = {{
    .size = 2,
    .basis_len = 1,
    .basis = (uint64_t []){0ull, 0ull, 1ull},
    .shift = (uint64_t []){0ull, 0ull, 0ull}},
{
    .size = 2,
    .basis_len = 1,
    .basis = (uint64_t []){0ull, 0ull, 6ull},
    .shift = (uint64_t []){0ull, 0ull, 0ull}},
{
    .size = 2,
    .basis_len = 1,
    .basis = (uint64_t []){0ull, 0ull, 360ull},
    .shift = (uint64_t []){0ull, 0ull, 0ull}},
{
    .size = 2,
    .basis_len = 1,
    .basis = (uint64_t []){0ull, 0ull, 18245760ull},
    .shift = (uint64_t []){0ull, 0ull, 0ull}},
{
    .size = 2,
    .basis_len = 1,
    .basis = (uint64_t []){0ull, 0ull, 78274681211224091ull},
    .shift = (uint64_t []){0ull, 0ull, 0ull}},
{
    .size = 2,
    .basis_len = 1,
    .basis = (uint64_t []){0ull, 0ull, 8244804748658156366ull},
    .shift = (uint64_t []){0ull, 0ull, 0ull}},
{
    .size = 2,
    .basis_len = 1,
    .basis = (uint64_t []){0ull, 0ull, 8749050644146709186ull},
    .shift = (uint64_t []){0ull, 0ull, 0ull}},
{
    .size = 2,
    .basis_len = 1,
    .basis = (uint64_t []){0ull, 0ull, 1085473640648473244ull},
    .shift = (uint64_t []){0ull, 0ull, 0ull}},
{
    .size = 2,
    .basis_len = 1,
    .basis = (uint64_t []){0ull, 0ull, 7640059558905957125ull},
    .shift = (uint64_t []){0ull, 0ull, 0ull}},
{
    .size = 2,
    .basis_len = 1,
    .basis = (uint64_t []){0ull, 0ull, 15191009085390685913ull},
    .shift = (uint64_t []){0ull, 0ull, 0ull}},
{
    .size = 2,
    .basis_len = 1,
    .basis = (uint64_t []){0ull, 0ull, 7878391778441664005ull},
    .shift = (uint64_t []){0ull, 0ull, 0ull}},
{
    .size = 2,
    .basis_len = 1,
    .basis = (uint64_t []){0ull, 0ull, 15027479999479429674ull},
    .shift = (uint64_t []){0ull, 0ull, 0ull}},
{
    .size = 2,
    .basis_len = 1,
    .basis = (uint64_t []){0ull, 0ull, 45258276057990961ull},
    .shift = (uint64_t []){0ull, 0ull, 0ull}},
{
    .size = 2,
    .basis_len = 1,
    .basis = (uint64_t []){0ull, 0ull, 4327488829911782419ull},
    .shift = (uint64_t []){0ull, 0ull, 0ull}}},
    .multilicheck_repetitions = 1,
    .num_ldt_instances = 1,
    .fri_query_repetitions = 381,
    .fri_interactive_repetitions = 1,
    .fri_num_reductions = 14,
    .max_ldt_tested_degree_bound = 16384,
    .fri_final_polynomial_degree_bound = 1,
    .fri_localization_parameters = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    .field_zero = {0},
    .field_one = {0, 0, 1}};

