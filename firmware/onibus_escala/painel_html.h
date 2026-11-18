// Página do painel servida pelo ESP32 (http://192.168.4.1). Sem dependências externas: funciona sem internet.
#pragma once

#include <pgmspace.h>

static const char PAINEL_HTML[] PROGMEM = R"HTML(<!doctype html>
<html lang="pt-BR">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<meta name="theme-color" content="#121417">
<title>Ônibus-escala · Ponto Falante</title>
<style>
:root{--fundo:#0e1013;--sup:#171a1f;--sup2:#22262d;--texto:#eeebe4;--texto2:#b4b8bf;--traco:#2d3138;
--prim:#d8ef88;--prim-t:#121417;--ambar:#ffb21e;--ok:#3ccf7a;--aviso:#ffb21e;--critico:#ff6b6b;--foco:#ffcc00}
*{box-sizing:border-box}
body{margin:0;background:var(--fundo);color:var(--texto);font:16px/1.4 system-ui,-apple-system,"Segoe UI",Roboto,sans-serif}
header{display:flex;align-items:center;justify-content:space-between;padding:14px 16px;border-bottom:1px solid var(--traco)}
header h1{font-size:18px;margin:0}header small{display:block;color:var(--texto2);font-size:13px;font-weight:400}
.con{display:flex;align-items:center;gap:6px;font-size:13px;color:var(--texto2)}
.con i{width:10px;height:10px;border-radius:50%;background:var(--critico)}.con.ok i{background:var(--ok)}
main{max-width:560px;margin:0 auto;padding:12px 12px 32px;display:grid;gap:12px}
section{background:var(--sup);border:1px solid var(--traco);border-radius:14px;padding:14px}
h2{font-size:13px;letter-spacing:.06em;text-transform:uppercase;color:var(--texto2);margin:0 0 10px}
.letreiro{background:#0b0c0e;border:1px solid var(--traco);border-radius:10px;padding:10px 12px;
font:700 28px/1.1 ui-monospace,Menlo,Consolas,monospace;color:var(--ambar);letter-spacing:.04em}
.letreiro.parado{color:var(--critico)}.letreiro.reduz{color:var(--aviso)}.letreiro.segue{color:var(--ok)}
.linha{display:flex;justify-content:space-between;align-items:baseline;gap:8px;margin-top:10px}
.dist{font:700 34px/1 ui-monospace,Menlo,Consolas,monospace}.dist small{font-size:16px;color:var(--texto2)}
.prob{display:grid;grid-template-columns:96px 1fr 44px;gap:6px 8px;align-items:center;margin-top:12px;font-size:14px}
.barra{height:12px;background:var(--sup2);border-radius:6px;overflow:hidden}
.barra b{display:block;height:100%;width:0;background:var(--texto2);transition:width .2s}
.prob .vence{color:var(--texto);font-weight:700}.prob .vence+.barra b{background:var(--prim)}
.nota{color:var(--texto2);font-size:13px;margin:8px 0 0}
.seg{display:grid;grid-auto-flow:column;grid-auto-columns:1fr;gap:6px}
button,.botao{font:inherit;font-weight:600;color:var(--texto);background:var(--sup2);border:1px solid var(--traco);
border-radius:10px;padding:12px;min-height:48px;cursor:pointer;touch-action:manipulation;text-align:center;text-decoration:none}
button[aria-pressed=true]{background:var(--prim);color:var(--prim-t);border-color:var(--prim)}
button:focus-visible,.botao:focus-visible{outline:3px solid var(--foco);outline-offset:2px}
button:disabled{opacity:.45;cursor:not-allowed}
.pad{display:grid;grid-template-columns:repeat(3,1fr);gap:8px;margin-top:10px;user-select:none;-webkit-user-select:none}
.pad button{font-size:24px;min-height:64px}.pad .vazio{visibility:hidden}
.pad button.ativo{background:var(--prim);color:var(--prim-t)}
.frase{background:var(--sup2);border-radius:10px;padding:12px;min-height:48px;margin-top:10px}
.acoes{display:flex;gap:8px;flex-wrap:wrap;margin-top:10px}.acoes>*{flex:1}
dl{display:grid;grid-template-columns:auto 1fr;gap:4px 12px;margin:0;font-size:14px}dt{color:var(--texto2)}dd{margin:0}
.sr{position:absolute;width:1px;height:1px;overflow:hidden;clip:rect(0 0 0 0)}
</style>
</head>
<body>
<header>
  <h1>Ônibus-escala<small>Ponto Falante · Maker Lab</small></h1>
  <div class="con" id="con"><i></i><span>sem conexão</span></div>
</header>
<main>
  <section aria-labelledby="t-agora">
    <h2 id="t-agora">Agora</h2>
    <div class="letreiro" id="estado" aria-live="polite">...</div>
    <div class="linha"><div class="dist" id="dist">--<small> cm</small></div><div id="motor" class="nota"></div></div>
    <div class="prob" id="prob" aria-label="Probabilidades da IA"></div>
    <p class="nota" id="ia-nota"></p>
  </section>

  <section aria-labelledby="t-modo">
    <h2 id="t-modo">Modo</h2>
    <div class="seg">
      <button id="m-manual" aria-pressed="false" onclick="post('/modo?m=manual')">Manual</button>
      <button id="m-autonomo" aria-pressed="false" onclick="post('/modo?m=autonomo')">Autônomo</button>
    </div>
    <div class="pad" id="pad" aria-label="Direção (segure para andar)">
      <span class="vazio"></span><button data-d="frente" aria-label="Frente">▲</button><span class="vazio"></span>
      <button data-d="esquerda" aria-label="Esquerda">◀</button><button data-d="parar" aria-label="Parar">■</button><button data-d="direita" aria-label="Direita">▶</button>
      <span class="vazio"></span><button data-d="re" aria-label="Ré">▼</button><span class="vazio"></span>
    </div>
    <p class="nota">Segure a seta para andar; ao soltar, o carrinho para. Setas do teclado também funcionam.</p>
    <div class="acoes"><button onclick="post('/buzina')">Buzina</button></div>
  </section>

  <section aria-labelledby="t-letreiro">
    <h2 id="t-letreiro">Letreiro</h2>
    <div class="seg">
      <button id="l-automatico" aria-pressed="false" onclick="post('/letreiro?m=automatico')">Automático</button>
      <button id="l-ligado" aria-pressed="false" onclick="post('/letreiro?m=ligado')">Ligado</button>
      <button id="l-desligado" aria-pressed="false" onclick="post('/letreiro?m=desligado')">Desligado</button>
    </div>
    <p class="nota" id="luz">luz --</p>
  </section>

  <section aria-labelledby="t-mvp">
    <h2 id="t-mvp">Ponto Falante (MVP)</h2>
    <dl id="mvp"></dl>
    <div class="acoes">
      <button id="consultar" onclick="post('/consultar')">Próximos ônibus do ponto</button>
      <button id="ouvir" onclick="ouvir()">Ouvir</button>
    </div>
    <div class="frase" id="frase" aria-live="polite">Nenhuma consulta ainda.</div>
  </section>

  <section aria-labelledby="t-coleta">
    <h2 id="t-coleta">Coleta de dados</h2>
    <p class="nota" id="coleta">--</p>
    <div class="acoes">
      <button id="gravar" onclick="post('/gravar?ligar='+(S&&S.gravando?0:1))">Gravar</button>
      <a class="botao" href="/coleta.csv" download="coleta.csv">Baixar CSV</a>
    </div>
    <p class="nota">Grava a distância já filtrada a 10 Hz, no formato do dataset. Renomeie para classe.N.csv e coloque em dataset/csv.</p>
  </section>
</main>
<script>
const $=id=>document.getElementById(id);
const CLASSES=['livre','aproximando','obstáculo'];
let S=null,falhas=0,versaoFrase=-1,movendo=null,timer=null;
const br=(v,c)=>v.toFixed(c).replace('.',',');

function post(url){return fetch(url,{method:'POST'}).then(atualizar).catch(()=>{});}

function render(s){
  S=s;
  const e=$('estado');
  e.textContent=s.estado;
  e.className='letreiro '+({SEGUINDO:'segue',REDUZINDO:'reduz',PARADO:'parado',FREIO:'parado'}[s.estado]||'');
  $('dist').innerHTML=(s.distancia>=199.9?'sem eco':br(s.distancia,1))+'<small> cm</small>';
  $('motor').textContent='motor: '+s.motor+(s.velocidade?' · '+s.velocidade:'');
  const p=$('prob');p.innerHTML='';
  s.prob.forEach((v,i)=>{
    const l=document.createElement('span');l.textContent=CLASSES[i];
    if(s.janela_pronta&&i===['livre','aproximando','obstaculo'].indexOf(s.classe))l.className='vence';
    const b=document.createElement('div');b.className='barra';b.innerHTML='<b style="width:'+Math.round(v*100)+'%"></b>';
    const t=document.createElement('span');t.textContent=Math.round(v*100)+'%';
    p.append(l,b,t);
  });
  $('ia-nota').textContent=s.janela_pronta?'Janela de 1,5 s, nova decisão a cada 0,2 s. Abaixo de 60% de confiança, mantém a decisão anterior.':'Enchendo a janela (1,5 s)...';
  $('m-manual').setAttribute('aria-pressed',s.modo==='manual');
  $('m-autonomo').setAttribute('aria-pressed',s.modo==='autonomo');
  ['automatico','ligado','desligado'].forEach(m=>$('l-'+m).setAttribute('aria-pressed',s.letreiro_modo===m));
  $('luz').textContent='luz '+s.luz+' de 4095 · letreiro '+(s.letreiro?'aceso':'apagado')+' (acende abaixo de 1200, apaga acima de 1600)';
  const m=s.mvp,dl=$('mvp');
  const itens=[['Ponto',m.ponto],['Internet',m.configurado?(m.wifi?'conectado · '+m.ip:'conectando...'):'desligada (credenciais.h)'],
    ['Relógio',m.hora?'sincronizado':'aguardando'],['Último envio',m.ha_s<0?'nenhum':m.tipo+' · HTTP '+m.codigo+' · há '+m.ha_s+' s'],
    ['Aceitos','' + m.enviados]];
  dl.innerHTML='';itens.forEach(([k,v])=>{const dt=document.createElement('dt');dt.textContent=k;const dd=document.createElement('dd');dd.textContent=v;dl.append(dt,dd);});
  $('consultar').disabled=m.enviando;
  if(m.versao!==versaoFrase&&m.frase){$('frase').textContent=m.frase;if(versaoFrase>=0)ouvir();}
  versaoFrase=m.versao;
  $('coleta').textContent=(s.gravando?'Gravando: ':'Parado: ')+s.amostras+' de '+s.amostras_max+' amostras ('+br(s.amostras/10,1)+' s)';
  $('gravar').textContent=s.gravando?'Parar gravação':'Gravar';
}

function atualizar(){
  return fetch('/estado',{cache:'no-store'}).then(r=>r.json()).then(s=>{falhas=0;render(s);con(true);})
    .catch(()=>{if(++falhas>2)con(false);});
}
function con(ok){$('con').className='con'+(ok?' ok':'');$('con').lastChild.textContent=ok?'conectado':'sem conexão';}

function ouvir(){
  const t=$('frase').textContent;if(!('speechSynthesis' in window)||!t)return;
  speechSynthesis.cancel();const u=new SpeechSynthesisUtterance(t);u.lang='pt-BR';speechSynthesis.speak(u);
}

function mover(d){
  parar(false);movendo=d;
  document.querySelectorAll('.pad button').forEach(b=>b.classList.toggle('ativo',b.dataset.d===d));
  const envia=()=>fetch('/mover?d='+d,{method:'POST'}).catch(()=>{});
  envia();timer=setInterval(envia,200);
}
function parar(enviar=true){
  if(timer)clearInterval(timer);timer=null;
  document.querySelectorAll('.pad button').forEach(b=>b.classList.remove('ativo'));
  if(enviar&&movendo)fetch('/mover?d=parar',{method:'POST'}).catch(()=>{});
  movendo=null;
}
document.querySelectorAll('.pad button').forEach(b=>{
  b.addEventListener('pointerdown',ev=>{ev.preventDefault();b.setPointerCapture(ev.pointerId);b.dataset.d==='parar'?parar():mover(b.dataset.d);});
  ['pointerup','pointercancel','lostpointercapture'].forEach(t=>b.addEventListener(t,()=>parar()));
  b.addEventListener('contextmenu',ev=>ev.preventDefault());
});
const TECLAS={ArrowUp:'frente',ArrowDown:'re',ArrowLeft:'esquerda',ArrowRight:'direita'};
addEventListener('keydown',ev=>{const d=TECLAS[ev.key];if(d&&!ev.repeat){ev.preventDefault();mover(d);}});
addEventListener('keyup',ev=>{if(TECLAS[ev.key])parar();});
addEventListener('blur',()=>parar());

atualizar();setInterval(atualizar,300);
</script>
</body>
</html>
)HTML";
