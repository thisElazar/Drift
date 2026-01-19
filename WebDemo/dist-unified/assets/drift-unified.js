var Sg=Object.defineProperty;var Tg=(i,t,e)=>t in i?Sg(i,t,{enumerable:!0,configurable:!0,writable:!0,value:e}):i[t]=e;var Vn=(i,t,e)=>Tg(i,typeof t!="symbol"?t+"":t,e);(function(){const t=document.createElement("link").relList;if(t&&t.supports&&t.supports("modulepreload"))return;for(const r of document.querySelectorAll('link[rel="modulepreload"]'))n(r);new MutationObserver(r=>{for(const s of r)if(s.type==="childList")for(const o of s.addedNodes)o.tagName==="LINK"&&o.rel==="modulepreload"&&n(o)}).observe(document,{childList:!0,subtree:!0});function e(r){const s={};return r.integrity&&(s.integrity=r.integrity),r.referrerPolicy&&(s.referrerPolicy=r.referrerPolicy),r.crossOrigin==="use-credentials"?s.credentials="include":r.crossOrigin==="anonymous"?s.credentials="omit":s.credentials="same-origin",s}function n(r){if(r.ep)return;r.ep=!0;const s=e(r);fetch(r.href,s)}})();/**
 * @license
 * Copyright 2010-2023 Three.js Authors
 * SPDX-License-Identifier: MIT
 */const Ol="160",wi={ROTATE:0,DOLLY:1,PAN:2},br={ROTATE:0,PAN:1,DOLLY_PAN:2,DOLLY_ROTATE:3},Mg=0,ru=1,wg=2,Wf=1,bg=2,oi=3,ui=0,vn=1,_n=2,Di=0,ts=1,su=2,ou=3,au=4,Ag=5,nr=100,Rg=101,Cg=102,cu=103,lu=104,Ig=200,Pg=201,Dg=202,Lg=203,rl=204,sl=205,Ng=206,Ug=207,Og=208,Fg=209,Vg=210,Bg=211,kg=212,Hg=213,zg=214,Gg=0,Wg=1,qg=2,xa=3,Xg=4,jg=5,Yg=6,$g=7,qf=0,Kg=1,Qg=2,Li=0,Zg=1,Jg=2,t_=3,Xf=4,e_=5,n_=6,jf=300,rs=301,ss=302,ol=303,al=304,Ga=306,cl=1e3,Gn=1001,ll=1002,pn=1003,hu=1004,dc=1005,Pn=1006,i_=1007,to=1008,Ni=1009,r_=1010,s_=1011,Fl=1012,Yf=1013,Ri=1014,Ci=1015,eo=1016,$f=1017,Kf=1018,or=1020,o_=1021,Wn=1023,a_=1024,c_=1025,ar=1026,os=1027,l_=1028,Qf=1029,h_=1030,Zf=1031,Jf=1033,fc=33776,pc=33777,mc=33778,gc=33779,uu=35840,du=35841,fu=35842,pu=35843,tp=36196,mu=37492,gu=37496,_u=37808,vu=37809,yu=37810,Eu=37811,xu=37812,Su=37813,Tu=37814,Mu=37815,wu=37816,bu=37817,Au=37818,Ru=37819,Cu=37820,Iu=37821,_c=36492,Pu=36494,Du=36495,u_=36283,Lu=36284,Nu=36285,Uu=36286,ep=3e3,cr=3001,d_=3200,f_=3201,np=0,p_=1,Nn="",Ze="srgb",di="srgb-linear",Vl="display-p3",Wa="display-p3-linear",Sa="linear",we="srgb",Ta="rec709",Ma="p3",Ar=7680,Ou=519,m_=512,g_=513,__=514,ip=515,v_=516,y_=517,E_=518,x_=519,hl=35044,Fu="300 es",ul=1035,li=2e3,wa=2001;class gr{addEventListener(t,e){this._listeners===void 0&&(this._listeners={});const n=this._listeners;n[t]===void 0&&(n[t]=[]),n[t].indexOf(e)===-1&&n[t].push(e)}hasEventListener(t,e){if(this._listeners===void 0)return!1;const n=this._listeners;return n[t]!==void 0&&n[t].indexOf(e)!==-1}removeEventListener(t,e){if(this._listeners===void 0)return;const r=this._listeners[t];if(r!==void 0){const s=r.indexOf(e);s!==-1&&r.splice(s,1)}}dispatchEvent(t){if(this._listeners===void 0)return;const n=this._listeners[t.type];if(n!==void 0){t.target=this;const r=n.slice(0);for(let s=0,o=r.length;s<o;s++)r[s].call(this,t);t.target=null}}}const on=["00","01","02","03","04","05","06","07","08","09","0a","0b","0c","0d","0e","0f","10","11","12","13","14","15","16","17","18","19","1a","1b","1c","1d","1e","1f","20","21","22","23","24","25","26","27","28","29","2a","2b","2c","2d","2e","2f","30","31","32","33","34","35","36","37","38","39","3a","3b","3c","3d","3e","3f","40","41","42","43","44","45","46","47","48","49","4a","4b","4c","4d","4e","4f","50","51","52","53","54","55","56","57","58","59","5a","5b","5c","5d","5e","5f","60","61","62","63","64","65","66","67","68","69","6a","6b","6c","6d","6e","6f","70","71","72","73","74","75","76","77","78","79","7a","7b","7c","7d","7e","7f","80","81","82","83","84","85","86","87","88","89","8a","8b","8c","8d","8e","8f","90","91","92","93","94","95","96","97","98","99","9a","9b","9c","9d","9e","9f","a0","a1","a2","a3","a4","a5","a6","a7","a8","a9","aa","ab","ac","ad","ae","af","b0","b1","b2","b3","b4","b5","b6","b7","b8","b9","ba","bb","bc","bd","be","bf","c0","c1","c2","c3","c4","c5","c6","c7","c8","c9","ca","cb","cc","cd","ce","cf","d0","d1","d2","d3","d4","d5","d6","d7","d8","d9","da","db","dc","dd","de","df","e0","e1","e2","e3","e4","e5","e6","e7","e8","e9","ea","eb","ec","ed","ee","ef","f0","f1","f2","f3","f4","f5","f6","f7","f8","f9","fa","fb","fc","fd","fe","ff"],fa=Math.PI/180,dl=180/Math.PI;function Ui(){const i=Math.random()*4294967295|0,t=Math.random()*4294967295|0,e=Math.random()*4294967295|0,n=Math.random()*4294967295|0;return(on[i&255]+on[i>>8&255]+on[i>>16&255]+on[i>>24&255]+"-"+on[t&255]+on[t>>8&255]+"-"+on[t>>16&15|64]+on[t>>24&255]+"-"+on[e&63|128]+on[e>>8&255]+"-"+on[e>>16&255]+on[e>>24&255]+on[n&255]+on[n>>8&255]+on[n>>16&255]+on[n>>24&255]).toLowerCase()}function mn(i,t,e){return Math.max(t,Math.min(e,i))}function S_(i,t){return(i%t+t)%t}function vc(i,t,e){return(1-e)*i+e*t}function Vu(i){return(i&i-1)===0&&i!==0}function fl(i){return Math.pow(2,Math.floor(Math.log(i)/Math.LN2))}function ai(i,t){switch(t.constructor){case Float32Array:return i;case Uint32Array:return i/4294967295;case Uint16Array:return i/65535;case Uint8Array:return i/255;case Int32Array:return Math.max(i/2147483647,-1);case Int16Array:return Math.max(i/32767,-1);case Int8Array:return Math.max(i/127,-1);default:throw new Error("Invalid component type.")}}function xe(i,t){switch(t.constructor){case Float32Array:return i;case Uint32Array:return Math.round(i*4294967295);case Uint16Array:return Math.round(i*65535);case Uint8Array:return Math.round(i*255);case Int32Array:return Math.round(i*2147483647);case Int16Array:return Math.round(i*32767);case Int8Array:return Math.round(i*127);default:throw new Error("Invalid component type.")}}const T_={DEG2RAD:fa};class Vt{constructor(t=0,e=0){Vt.prototype.isVector2=!0,this.x=t,this.y=e}get width(){return this.x}set width(t){this.x=t}get height(){return this.y}set height(t){this.y=t}set(t,e){return this.x=t,this.y=e,this}setScalar(t){return this.x=t,this.y=t,this}setX(t){return this.x=t,this}setY(t){return this.y=t,this}setComponent(t,e){switch(t){case 0:this.x=e;break;case 1:this.y=e;break;default:throw new Error("index is out of range: "+t)}return this}getComponent(t){switch(t){case 0:return this.x;case 1:return this.y;default:throw new Error("index is out of range: "+t)}}clone(){return new this.constructor(this.x,this.y)}copy(t){return this.x=t.x,this.y=t.y,this}add(t){return this.x+=t.x,this.y+=t.y,this}addScalar(t){return this.x+=t,this.y+=t,this}addVectors(t,e){return this.x=t.x+e.x,this.y=t.y+e.y,this}addScaledVector(t,e){return this.x+=t.x*e,this.y+=t.y*e,this}sub(t){return this.x-=t.x,this.y-=t.y,this}subScalar(t){return this.x-=t,this.y-=t,this}subVectors(t,e){return this.x=t.x-e.x,this.y=t.y-e.y,this}multiply(t){return this.x*=t.x,this.y*=t.y,this}multiplyScalar(t){return this.x*=t,this.y*=t,this}divide(t){return this.x/=t.x,this.y/=t.y,this}divideScalar(t){return this.multiplyScalar(1/t)}applyMatrix3(t){const e=this.x,n=this.y,r=t.elements;return this.x=r[0]*e+r[3]*n+r[6],this.y=r[1]*e+r[4]*n+r[7],this}min(t){return this.x=Math.min(this.x,t.x),this.y=Math.min(this.y,t.y),this}max(t){return this.x=Math.max(this.x,t.x),this.y=Math.max(this.y,t.y),this}clamp(t,e){return this.x=Math.max(t.x,Math.min(e.x,this.x)),this.y=Math.max(t.y,Math.min(e.y,this.y)),this}clampScalar(t,e){return this.x=Math.max(t,Math.min(e,this.x)),this.y=Math.max(t,Math.min(e,this.y)),this}clampLength(t,e){const n=this.length();return this.divideScalar(n||1).multiplyScalar(Math.max(t,Math.min(e,n)))}floor(){return this.x=Math.floor(this.x),this.y=Math.floor(this.y),this}ceil(){return this.x=Math.ceil(this.x),this.y=Math.ceil(this.y),this}round(){return this.x=Math.round(this.x),this.y=Math.round(this.y),this}roundToZero(){return this.x=Math.trunc(this.x),this.y=Math.trunc(this.y),this}negate(){return this.x=-this.x,this.y=-this.y,this}dot(t){return this.x*t.x+this.y*t.y}cross(t){return this.x*t.y-this.y*t.x}lengthSq(){return this.x*this.x+this.y*this.y}length(){return Math.sqrt(this.x*this.x+this.y*this.y)}manhattanLength(){return Math.abs(this.x)+Math.abs(this.y)}normalize(){return this.divideScalar(this.length()||1)}angle(){return Math.atan2(-this.y,-this.x)+Math.PI}angleTo(t){const e=Math.sqrt(this.lengthSq()*t.lengthSq());if(e===0)return Math.PI/2;const n=this.dot(t)/e;return Math.acos(mn(n,-1,1))}distanceTo(t){return Math.sqrt(this.distanceToSquared(t))}distanceToSquared(t){const e=this.x-t.x,n=this.y-t.y;return e*e+n*n}manhattanDistanceTo(t){return Math.abs(this.x-t.x)+Math.abs(this.y-t.y)}setLength(t){return this.normalize().multiplyScalar(t)}lerp(t,e){return this.x+=(t.x-this.x)*e,this.y+=(t.y-this.y)*e,this}lerpVectors(t,e,n){return this.x=t.x+(e.x-t.x)*n,this.y=t.y+(e.y-t.y)*n,this}equals(t){return t.x===this.x&&t.y===this.y}fromArray(t,e=0){return this.x=t[e],this.y=t[e+1],this}toArray(t=[],e=0){return t[e]=this.x,t[e+1]=this.y,t}fromBufferAttribute(t,e){return this.x=t.getX(e),this.y=t.getY(e),this}rotateAround(t,e){const n=Math.cos(e),r=Math.sin(e),s=this.x-t.x,o=this.y-t.y;return this.x=s*n-o*r+t.x,this.y=s*r+o*n+t.y,this}random(){return this.x=Math.random(),this.y=Math.random(),this}*[Symbol.iterator](){yield this.x,yield this.y}}class ae{constructor(t,e,n,r,s,o,a,c,h){ae.prototype.isMatrix3=!0,this.elements=[1,0,0,0,1,0,0,0,1],t!==void 0&&this.set(t,e,n,r,s,o,a,c,h)}set(t,e,n,r,s,o,a,c,h){const u=this.elements;return u[0]=t,u[1]=r,u[2]=a,u[3]=e,u[4]=s,u[5]=c,u[6]=n,u[7]=o,u[8]=h,this}identity(){return this.set(1,0,0,0,1,0,0,0,1),this}copy(t){const e=this.elements,n=t.elements;return e[0]=n[0],e[1]=n[1],e[2]=n[2],e[3]=n[3],e[4]=n[4],e[5]=n[5],e[6]=n[6],e[7]=n[7],e[8]=n[8],this}extractBasis(t,e,n){return t.setFromMatrix3Column(this,0),e.setFromMatrix3Column(this,1),n.setFromMatrix3Column(this,2),this}setFromMatrix4(t){const e=t.elements;return this.set(e[0],e[4],e[8],e[1],e[5],e[9],e[2],e[6],e[10]),this}multiply(t){return this.multiplyMatrices(this,t)}premultiply(t){return this.multiplyMatrices(t,this)}multiplyMatrices(t,e){const n=t.elements,r=e.elements,s=this.elements,o=n[0],a=n[3],c=n[6],h=n[1],u=n[4],d=n[7],f=n[2],p=n[5],y=n[8],E=r[0],_=r[3],m=r[6],C=r[1],w=r[4],b=r[7],N=r[2],U=r[5],T=r[8];return s[0]=o*E+a*C+c*N,s[3]=o*_+a*w+c*U,s[6]=o*m+a*b+c*T,s[1]=h*E+u*C+d*N,s[4]=h*_+u*w+d*U,s[7]=h*m+u*b+d*T,s[2]=f*E+p*C+y*N,s[5]=f*_+p*w+y*U,s[8]=f*m+p*b+y*T,this}multiplyScalar(t){const e=this.elements;return e[0]*=t,e[3]*=t,e[6]*=t,e[1]*=t,e[4]*=t,e[7]*=t,e[2]*=t,e[5]*=t,e[8]*=t,this}determinant(){const t=this.elements,e=t[0],n=t[1],r=t[2],s=t[3],o=t[4],a=t[5],c=t[6],h=t[7],u=t[8];return e*o*u-e*a*h-n*s*u+n*a*c+r*s*h-r*o*c}invert(){const t=this.elements,e=t[0],n=t[1],r=t[2],s=t[3],o=t[4],a=t[5],c=t[6],h=t[7],u=t[8],d=u*o-a*h,f=a*c-u*s,p=h*s-o*c,y=e*d+n*f+r*p;if(y===0)return this.set(0,0,0,0,0,0,0,0,0);const E=1/y;return t[0]=d*E,t[1]=(r*h-u*n)*E,t[2]=(a*n-r*o)*E,t[3]=f*E,t[4]=(u*e-r*c)*E,t[5]=(r*s-a*e)*E,t[6]=p*E,t[7]=(n*c-h*e)*E,t[8]=(o*e-n*s)*E,this}transpose(){let t;const e=this.elements;return t=e[1],e[1]=e[3],e[3]=t,t=e[2],e[2]=e[6],e[6]=t,t=e[5],e[5]=e[7],e[7]=t,this}getNormalMatrix(t){return this.setFromMatrix4(t).invert().transpose()}transposeIntoArray(t){const e=this.elements;return t[0]=e[0],t[1]=e[3],t[2]=e[6],t[3]=e[1],t[4]=e[4],t[5]=e[7],t[6]=e[2],t[7]=e[5],t[8]=e[8],this}setUvTransform(t,e,n,r,s,o,a){const c=Math.cos(s),h=Math.sin(s);return this.set(n*c,n*h,-n*(c*o+h*a)+o+t,-r*h,r*c,-r*(-h*o+c*a)+a+e,0,0,1),this}scale(t,e){return this.premultiply(yc.makeScale(t,e)),this}rotate(t){return this.premultiply(yc.makeRotation(-t)),this}translate(t,e){return this.premultiply(yc.makeTranslation(t,e)),this}makeTranslation(t,e){return t.isVector2?this.set(1,0,t.x,0,1,t.y,0,0,1):this.set(1,0,t,0,1,e,0,0,1),this}makeRotation(t){const e=Math.cos(t),n=Math.sin(t);return this.set(e,-n,0,n,e,0,0,0,1),this}makeScale(t,e){return this.set(t,0,0,0,e,0,0,0,1),this}equals(t){const e=this.elements,n=t.elements;for(let r=0;r<9;r++)if(e[r]!==n[r])return!1;return!0}fromArray(t,e=0){for(let n=0;n<9;n++)this.elements[n]=t[n+e];return this}toArray(t=[],e=0){const n=this.elements;return t[e]=n[0],t[e+1]=n[1],t[e+2]=n[2],t[e+3]=n[3],t[e+4]=n[4],t[e+5]=n[5],t[e+6]=n[6],t[e+7]=n[7],t[e+8]=n[8],t}clone(){return new this.constructor().fromArray(this.elements)}}const yc=new ae;function rp(i){for(let t=i.length-1;t>=0;--t)if(i[t]>=65535)return!0;return!1}function ba(i){return document.createElementNS("http://www.w3.org/1999/xhtml",i)}function M_(){const i=ba("canvas");return i.style.display="block",i}const Bu={};function Ys(i){i in Bu||(Bu[i]=!0,console.warn(i))}const ku=new ae().set(.8224621,.177538,0,.0331941,.9668058,0,.0170827,.0723974,.9105199),Hu=new ae().set(1.2249401,-.2249404,0,-.0420569,1.0420571,0,-.0196376,-.0786361,1.0982735),Lo={[di]:{transfer:Sa,primaries:Ta,toReference:i=>i,fromReference:i=>i},[Ze]:{transfer:we,primaries:Ta,toReference:i=>i.convertSRGBToLinear(),fromReference:i=>i.convertLinearToSRGB()},[Wa]:{transfer:Sa,primaries:Ma,toReference:i=>i.applyMatrix3(Hu),fromReference:i=>i.applyMatrix3(ku)},[Vl]:{transfer:we,primaries:Ma,toReference:i=>i.convertSRGBToLinear().applyMatrix3(Hu),fromReference:i=>i.applyMatrix3(ku).convertLinearToSRGB()}},w_=new Set([di,Wa]),ve={enabled:!0,_workingColorSpace:di,get workingColorSpace(){return this._workingColorSpace},set workingColorSpace(i){if(!w_.has(i))throw new Error(`Unsupported working color space, "${i}".`);this._workingColorSpace=i},convert:function(i,t,e){if(this.enabled===!1||t===e||!t||!e)return i;const n=Lo[t].toReference,r=Lo[e].fromReference;return r(n(i))},fromWorkingColorSpace:function(i,t){return this.convert(i,this._workingColorSpace,t)},toWorkingColorSpace:function(i,t){return this.convert(i,t,this._workingColorSpace)},getPrimaries:function(i){return Lo[i].primaries},getTransfer:function(i){return i===Nn?Sa:Lo[i].transfer}};function es(i){return i<.04045?i*.0773993808:Math.pow(i*.9478672986+.0521327014,2.4)}function Ec(i){return i<.0031308?i*12.92:1.055*Math.pow(i,.41666)-.055}let Rr;class sp{static getDataURL(t){if(/^data:/i.test(t.src)||typeof HTMLCanvasElement>"u")return t.src;let e;if(t instanceof HTMLCanvasElement)e=t;else{Rr===void 0&&(Rr=ba("canvas")),Rr.width=t.width,Rr.height=t.height;const n=Rr.getContext("2d");t instanceof ImageData?n.putImageData(t,0,0):n.drawImage(t,0,0,t.width,t.height),e=Rr}return e.width>2048||e.height>2048?(console.warn("THREE.ImageUtils.getDataURL: Image converted to jpg for performance reasons",t),e.toDataURL("image/jpeg",.6)):e.toDataURL("image/png")}static sRGBToLinear(t){if(typeof HTMLImageElement<"u"&&t instanceof HTMLImageElement||typeof HTMLCanvasElement<"u"&&t instanceof HTMLCanvasElement||typeof ImageBitmap<"u"&&t instanceof ImageBitmap){const e=ba("canvas");e.width=t.width,e.height=t.height;const n=e.getContext("2d");n.drawImage(t,0,0,t.width,t.height);const r=n.getImageData(0,0,t.width,t.height),s=r.data;for(let o=0;o<s.length;o++)s[o]=es(s[o]/255)*255;return n.putImageData(r,0,0),e}else if(t.data){const e=t.data.slice(0);for(let n=0;n<e.length;n++)e instanceof Uint8Array||e instanceof Uint8ClampedArray?e[n]=Math.floor(es(e[n]/255)*255):e[n]=es(e[n]);return{data:e,width:t.width,height:t.height}}else return console.warn("THREE.ImageUtils.sRGBToLinear(): Unsupported image type. No color space conversion applied."),t}}let b_=0;class op{constructor(t=null){this.isSource=!0,Object.defineProperty(this,"id",{value:b_++}),this.uuid=Ui(),this.data=t,this.version=0}set needsUpdate(t){t===!0&&this.version++}toJSON(t){const e=t===void 0||typeof t=="string";if(!e&&t.images[this.uuid]!==void 0)return t.images[this.uuid];const n={uuid:this.uuid,url:""},r=this.data;if(r!==null){let s;if(Array.isArray(r)){s=[];for(let o=0,a=r.length;o<a;o++)r[o].isDataTexture?s.push(xc(r[o].image)):s.push(xc(r[o]))}else s=xc(r);n.url=s}return e||(t.images[this.uuid]=n),n}}function xc(i){return typeof HTMLImageElement<"u"&&i instanceof HTMLImageElement||typeof HTMLCanvasElement<"u"&&i instanceof HTMLCanvasElement||typeof ImageBitmap<"u"&&i instanceof ImageBitmap?sp.getDataURL(i):i.data?{data:Array.from(i.data),width:i.width,height:i.height,type:i.data.constructor.name}:(console.warn("THREE.Texture: Unable to serialize Texture."),{})}let A_=0;class En extends gr{constructor(t=En.DEFAULT_IMAGE,e=En.DEFAULT_MAPPING,n=Gn,r=Gn,s=Pn,o=to,a=Wn,c=Ni,h=En.DEFAULT_ANISOTROPY,u=Nn){super(),this.isTexture=!0,Object.defineProperty(this,"id",{value:A_++}),this.uuid=Ui(),this.name="",this.source=new op(t),this.mipmaps=[],this.mapping=e,this.channel=0,this.wrapS=n,this.wrapT=r,this.magFilter=s,this.minFilter=o,this.anisotropy=h,this.format=a,this.internalFormat=null,this.type=c,this.offset=new Vt(0,0),this.repeat=new Vt(1,1),this.center=new Vt(0,0),this.rotation=0,this.matrixAutoUpdate=!0,this.matrix=new ae,this.generateMipmaps=!0,this.premultiplyAlpha=!1,this.flipY=!0,this.unpackAlignment=4,typeof u=="string"?this.colorSpace=u:(Ys("THREE.Texture: Property .encoding has been replaced by .colorSpace."),this.colorSpace=u===cr?Ze:Nn),this.userData={},this.version=0,this.onUpdate=null,this.isRenderTargetTexture=!1,this.needsPMREMUpdate=!1}get image(){return this.source.data}set image(t=null){this.source.data=t}updateMatrix(){this.matrix.setUvTransform(this.offset.x,this.offset.y,this.repeat.x,this.repeat.y,this.rotation,this.center.x,this.center.y)}clone(){return new this.constructor().copy(this)}copy(t){return this.name=t.name,this.source=t.source,this.mipmaps=t.mipmaps.slice(0),this.mapping=t.mapping,this.channel=t.channel,this.wrapS=t.wrapS,this.wrapT=t.wrapT,this.magFilter=t.magFilter,this.minFilter=t.minFilter,this.anisotropy=t.anisotropy,this.format=t.format,this.internalFormat=t.internalFormat,this.type=t.type,this.offset.copy(t.offset),this.repeat.copy(t.repeat),this.center.copy(t.center),this.rotation=t.rotation,this.matrixAutoUpdate=t.matrixAutoUpdate,this.matrix.copy(t.matrix),this.generateMipmaps=t.generateMipmaps,this.premultiplyAlpha=t.premultiplyAlpha,this.flipY=t.flipY,this.unpackAlignment=t.unpackAlignment,this.colorSpace=t.colorSpace,this.userData=JSON.parse(JSON.stringify(t.userData)),this.needsUpdate=!0,this}toJSON(t){const e=t===void 0||typeof t=="string";if(!e&&t.textures[this.uuid]!==void 0)return t.textures[this.uuid];const n={metadata:{version:4.6,type:"Texture",generator:"Texture.toJSON"},uuid:this.uuid,name:this.name,image:this.source.toJSON(t).uuid,mapping:this.mapping,channel:this.channel,repeat:[this.repeat.x,this.repeat.y],offset:[this.offset.x,this.offset.y],center:[this.center.x,this.center.y],rotation:this.rotation,wrap:[this.wrapS,this.wrapT],format:this.format,internalFormat:this.internalFormat,type:this.type,colorSpace:this.colorSpace,minFilter:this.minFilter,magFilter:this.magFilter,anisotropy:this.anisotropy,flipY:this.flipY,generateMipmaps:this.generateMipmaps,premultiplyAlpha:this.premultiplyAlpha,unpackAlignment:this.unpackAlignment};return Object.keys(this.userData).length>0&&(n.userData=this.userData),e||(t.textures[this.uuid]=n),n}dispose(){this.dispatchEvent({type:"dispose"})}transformUv(t){if(this.mapping!==jf)return t;if(t.applyMatrix3(this.matrix),t.x<0||t.x>1)switch(this.wrapS){case cl:t.x=t.x-Math.floor(t.x);break;case Gn:t.x=t.x<0?0:1;break;case ll:Math.abs(Math.floor(t.x)%2)===1?t.x=Math.ceil(t.x)-t.x:t.x=t.x-Math.floor(t.x);break}if(t.y<0||t.y>1)switch(this.wrapT){case cl:t.y=t.y-Math.floor(t.y);break;case Gn:t.y=t.y<0?0:1;break;case ll:Math.abs(Math.floor(t.y)%2)===1?t.y=Math.ceil(t.y)-t.y:t.y=t.y-Math.floor(t.y);break}return this.flipY&&(t.y=1-t.y),t}set needsUpdate(t){t===!0&&(this.version++,this.source.needsUpdate=!0)}get encoding(){return Ys("THREE.Texture: Property .encoding has been replaced by .colorSpace."),this.colorSpace===Ze?cr:ep}set encoding(t){Ys("THREE.Texture: Property .encoding has been replaced by .colorSpace."),this.colorSpace=t===cr?Ze:Nn}}En.DEFAULT_IMAGE=null;En.DEFAULT_MAPPING=jf;En.DEFAULT_ANISOTROPY=1;class Xe{constructor(t=0,e=0,n=0,r=1){Xe.prototype.isVector4=!0,this.x=t,this.y=e,this.z=n,this.w=r}get width(){return this.z}set width(t){this.z=t}get height(){return this.w}set height(t){this.w=t}set(t,e,n,r){return this.x=t,this.y=e,this.z=n,this.w=r,this}setScalar(t){return this.x=t,this.y=t,this.z=t,this.w=t,this}setX(t){return this.x=t,this}setY(t){return this.y=t,this}setZ(t){return this.z=t,this}setW(t){return this.w=t,this}setComponent(t,e){switch(t){case 0:this.x=e;break;case 1:this.y=e;break;case 2:this.z=e;break;case 3:this.w=e;break;default:throw new Error("index is out of range: "+t)}return this}getComponent(t){switch(t){case 0:return this.x;case 1:return this.y;case 2:return this.z;case 3:return this.w;default:throw new Error("index is out of range: "+t)}}clone(){return new this.constructor(this.x,this.y,this.z,this.w)}copy(t){return this.x=t.x,this.y=t.y,this.z=t.z,this.w=t.w!==void 0?t.w:1,this}add(t){return this.x+=t.x,this.y+=t.y,this.z+=t.z,this.w+=t.w,this}addScalar(t){return this.x+=t,this.y+=t,this.z+=t,this.w+=t,this}addVectors(t,e){return this.x=t.x+e.x,this.y=t.y+e.y,this.z=t.z+e.z,this.w=t.w+e.w,this}addScaledVector(t,e){return this.x+=t.x*e,this.y+=t.y*e,this.z+=t.z*e,this.w+=t.w*e,this}sub(t){return this.x-=t.x,this.y-=t.y,this.z-=t.z,this.w-=t.w,this}subScalar(t){return this.x-=t,this.y-=t,this.z-=t,this.w-=t,this}subVectors(t,e){return this.x=t.x-e.x,this.y=t.y-e.y,this.z=t.z-e.z,this.w=t.w-e.w,this}multiply(t){return this.x*=t.x,this.y*=t.y,this.z*=t.z,this.w*=t.w,this}multiplyScalar(t){return this.x*=t,this.y*=t,this.z*=t,this.w*=t,this}applyMatrix4(t){const e=this.x,n=this.y,r=this.z,s=this.w,o=t.elements;return this.x=o[0]*e+o[4]*n+o[8]*r+o[12]*s,this.y=o[1]*e+o[5]*n+o[9]*r+o[13]*s,this.z=o[2]*e+o[6]*n+o[10]*r+o[14]*s,this.w=o[3]*e+o[7]*n+o[11]*r+o[15]*s,this}divideScalar(t){return this.multiplyScalar(1/t)}setAxisAngleFromQuaternion(t){this.w=2*Math.acos(t.w);const e=Math.sqrt(1-t.w*t.w);return e<1e-4?(this.x=1,this.y=0,this.z=0):(this.x=t.x/e,this.y=t.y/e,this.z=t.z/e),this}setAxisAngleFromRotationMatrix(t){let e,n,r,s;const c=t.elements,h=c[0],u=c[4],d=c[8],f=c[1],p=c[5],y=c[9],E=c[2],_=c[6],m=c[10];if(Math.abs(u-f)<.01&&Math.abs(d-E)<.01&&Math.abs(y-_)<.01){if(Math.abs(u+f)<.1&&Math.abs(d+E)<.1&&Math.abs(y+_)<.1&&Math.abs(h+p+m-3)<.1)return this.set(1,0,0,0),this;e=Math.PI;const w=(h+1)/2,b=(p+1)/2,N=(m+1)/2,U=(u+f)/4,T=(d+E)/4,M=(y+_)/4;return w>b&&w>N?w<.01?(n=0,r=.707106781,s=.707106781):(n=Math.sqrt(w),r=U/n,s=T/n):b>N?b<.01?(n=.707106781,r=0,s=.707106781):(r=Math.sqrt(b),n=U/r,s=M/r):N<.01?(n=.707106781,r=.707106781,s=0):(s=Math.sqrt(N),n=T/s,r=M/s),this.set(n,r,s,e),this}let C=Math.sqrt((_-y)*(_-y)+(d-E)*(d-E)+(f-u)*(f-u));return Math.abs(C)<.001&&(C=1),this.x=(_-y)/C,this.y=(d-E)/C,this.z=(f-u)/C,this.w=Math.acos((h+p+m-1)/2),this}min(t){return this.x=Math.min(this.x,t.x),this.y=Math.min(this.y,t.y),this.z=Math.min(this.z,t.z),this.w=Math.min(this.w,t.w),this}max(t){return this.x=Math.max(this.x,t.x),this.y=Math.max(this.y,t.y),this.z=Math.max(this.z,t.z),this.w=Math.max(this.w,t.w),this}clamp(t,e){return this.x=Math.max(t.x,Math.min(e.x,this.x)),this.y=Math.max(t.y,Math.min(e.y,this.y)),this.z=Math.max(t.z,Math.min(e.z,this.z)),this.w=Math.max(t.w,Math.min(e.w,this.w)),this}clampScalar(t,e){return this.x=Math.max(t,Math.min(e,this.x)),this.y=Math.max(t,Math.min(e,this.y)),this.z=Math.max(t,Math.min(e,this.z)),this.w=Math.max(t,Math.min(e,this.w)),this}clampLength(t,e){const n=this.length();return this.divideScalar(n||1).multiplyScalar(Math.max(t,Math.min(e,n)))}floor(){return this.x=Math.floor(this.x),this.y=Math.floor(this.y),this.z=Math.floor(this.z),this.w=Math.floor(this.w),this}ceil(){return this.x=Math.ceil(this.x),this.y=Math.ceil(this.y),this.z=Math.ceil(this.z),this.w=Math.ceil(this.w),this}round(){return this.x=Math.round(this.x),this.y=Math.round(this.y),this.z=Math.round(this.z),this.w=Math.round(this.w),this}roundToZero(){return this.x=Math.trunc(this.x),this.y=Math.trunc(this.y),this.z=Math.trunc(this.z),this.w=Math.trunc(this.w),this}negate(){return this.x=-this.x,this.y=-this.y,this.z=-this.z,this.w=-this.w,this}dot(t){return this.x*t.x+this.y*t.y+this.z*t.z+this.w*t.w}lengthSq(){return this.x*this.x+this.y*this.y+this.z*this.z+this.w*this.w}length(){return Math.sqrt(this.x*this.x+this.y*this.y+this.z*this.z+this.w*this.w)}manhattanLength(){return Math.abs(this.x)+Math.abs(this.y)+Math.abs(this.z)+Math.abs(this.w)}normalize(){return this.divideScalar(this.length()||1)}setLength(t){return this.normalize().multiplyScalar(t)}lerp(t,e){return this.x+=(t.x-this.x)*e,this.y+=(t.y-this.y)*e,this.z+=(t.z-this.z)*e,this.w+=(t.w-this.w)*e,this}lerpVectors(t,e,n){return this.x=t.x+(e.x-t.x)*n,this.y=t.y+(e.y-t.y)*n,this.z=t.z+(e.z-t.z)*n,this.w=t.w+(e.w-t.w)*n,this}equals(t){return t.x===this.x&&t.y===this.y&&t.z===this.z&&t.w===this.w}fromArray(t,e=0){return this.x=t[e],this.y=t[e+1],this.z=t[e+2],this.w=t[e+3],this}toArray(t=[],e=0){return t[e]=this.x,t[e+1]=this.y,t[e+2]=this.z,t[e+3]=this.w,t}fromBufferAttribute(t,e){return this.x=t.getX(e),this.y=t.getY(e),this.z=t.getZ(e),this.w=t.getW(e),this}random(){return this.x=Math.random(),this.y=Math.random(),this.z=Math.random(),this.w=Math.random(),this}*[Symbol.iterator](){yield this.x,yield this.y,yield this.z,yield this.w}}class R_ extends gr{constructor(t=1,e=1,n={}){super(),this.isRenderTarget=!0,this.width=t,this.height=e,this.depth=1,this.scissor=new Xe(0,0,t,e),this.scissorTest=!1,this.viewport=new Xe(0,0,t,e);const r={width:t,height:e,depth:1};n.encoding!==void 0&&(Ys("THREE.WebGLRenderTarget: option.encoding has been replaced by option.colorSpace."),n.colorSpace=n.encoding===cr?Ze:Nn),n=Object.assign({generateMipmaps:!1,internalFormat:null,minFilter:Pn,depthBuffer:!0,stencilBuffer:!1,depthTexture:null,samples:0},n),this.texture=new En(r,n.mapping,n.wrapS,n.wrapT,n.magFilter,n.minFilter,n.format,n.type,n.anisotropy,n.colorSpace),this.texture.isRenderTargetTexture=!0,this.texture.flipY=!1,this.texture.generateMipmaps=n.generateMipmaps,this.texture.internalFormat=n.internalFormat,this.depthBuffer=n.depthBuffer,this.stencilBuffer=n.stencilBuffer,this.depthTexture=n.depthTexture,this.samples=n.samples}setSize(t,e,n=1){(this.width!==t||this.height!==e||this.depth!==n)&&(this.width=t,this.height=e,this.depth=n,this.texture.image.width=t,this.texture.image.height=e,this.texture.image.depth=n,this.dispose()),this.viewport.set(0,0,t,e),this.scissor.set(0,0,t,e)}clone(){return new this.constructor().copy(this)}copy(t){this.width=t.width,this.height=t.height,this.depth=t.depth,this.scissor.copy(t.scissor),this.scissorTest=t.scissorTest,this.viewport.copy(t.viewport),this.texture=t.texture.clone(),this.texture.isRenderTargetTexture=!0;const e=Object.assign({},t.texture.image);return this.texture.source=new op(e),this.depthBuffer=t.depthBuffer,this.stencilBuffer=t.stencilBuffer,t.depthTexture!==null&&(this.depthTexture=t.depthTexture.clone()),this.samples=t.samples,this}dispose(){this.dispatchEvent({type:"dispose"})}}class hr extends R_{constructor(t=1,e=1,n={}){super(t,e,n),this.isWebGLRenderTarget=!0}}class ap extends En{constructor(t=null,e=1,n=1,r=1){super(null),this.isDataArrayTexture=!0,this.image={data:t,width:e,height:n,depth:r},this.magFilter=pn,this.minFilter=pn,this.wrapR=Gn,this.generateMipmaps=!1,this.flipY=!1,this.unpackAlignment=1}}class C_ extends En{constructor(t=null,e=1,n=1,r=1){super(null),this.isData3DTexture=!0,this.image={data:t,width:e,height:n,depth:r},this.magFilter=pn,this.minFilter=pn,this.wrapR=Gn,this.generateMipmaps=!1,this.flipY=!1,this.unpackAlignment=1}}class ur{constructor(t=0,e=0,n=0,r=1){this.isQuaternion=!0,this._x=t,this._y=e,this._z=n,this._w=r}static slerpFlat(t,e,n,r,s,o,a){let c=n[r+0],h=n[r+1],u=n[r+2],d=n[r+3];const f=s[o+0],p=s[o+1],y=s[o+2],E=s[o+3];if(a===0){t[e+0]=c,t[e+1]=h,t[e+2]=u,t[e+3]=d;return}if(a===1){t[e+0]=f,t[e+1]=p,t[e+2]=y,t[e+3]=E;return}if(d!==E||c!==f||h!==p||u!==y){let _=1-a;const m=c*f+h*p+u*y+d*E,C=m>=0?1:-1,w=1-m*m;if(w>Number.EPSILON){const N=Math.sqrt(w),U=Math.atan2(N,m*C);_=Math.sin(_*U)/N,a=Math.sin(a*U)/N}const b=a*C;if(c=c*_+f*b,h=h*_+p*b,u=u*_+y*b,d=d*_+E*b,_===1-a){const N=1/Math.sqrt(c*c+h*h+u*u+d*d);c*=N,h*=N,u*=N,d*=N}}t[e]=c,t[e+1]=h,t[e+2]=u,t[e+3]=d}static multiplyQuaternionsFlat(t,e,n,r,s,o){const a=n[r],c=n[r+1],h=n[r+2],u=n[r+3],d=s[o],f=s[o+1],p=s[o+2],y=s[o+3];return t[e]=a*y+u*d+c*p-h*f,t[e+1]=c*y+u*f+h*d-a*p,t[e+2]=h*y+u*p+a*f-c*d,t[e+3]=u*y-a*d-c*f-h*p,t}get x(){return this._x}set x(t){this._x=t,this._onChangeCallback()}get y(){return this._y}set y(t){this._y=t,this._onChangeCallback()}get z(){return this._z}set z(t){this._z=t,this._onChangeCallback()}get w(){return this._w}set w(t){this._w=t,this._onChangeCallback()}set(t,e,n,r){return this._x=t,this._y=e,this._z=n,this._w=r,this._onChangeCallback(),this}clone(){return new this.constructor(this._x,this._y,this._z,this._w)}copy(t){return this._x=t.x,this._y=t.y,this._z=t.z,this._w=t.w,this._onChangeCallback(),this}setFromEuler(t,e=!0){const n=t._x,r=t._y,s=t._z,o=t._order,a=Math.cos,c=Math.sin,h=a(n/2),u=a(r/2),d=a(s/2),f=c(n/2),p=c(r/2),y=c(s/2);switch(o){case"XYZ":this._x=f*u*d+h*p*y,this._y=h*p*d-f*u*y,this._z=h*u*y+f*p*d,this._w=h*u*d-f*p*y;break;case"YXZ":this._x=f*u*d+h*p*y,this._y=h*p*d-f*u*y,this._z=h*u*y-f*p*d,this._w=h*u*d+f*p*y;break;case"ZXY":this._x=f*u*d-h*p*y,this._y=h*p*d+f*u*y,this._z=h*u*y+f*p*d,this._w=h*u*d-f*p*y;break;case"ZYX":this._x=f*u*d-h*p*y,this._y=h*p*d+f*u*y,this._z=h*u*y-f*p*d,this._w=h*u*d+f*p*y;break;case"YZX":this._x=f*u*d+h*p*y,this._y=h*p*d+f*u*y,this._z=h*u*y-f*p*d,this._w=h*u*d-f*p*y;break;case"XZY":this._x=f*u*d-h*p*y,this._y=h*p*d-f*u*y,this._z=h*u*y+f*p*d,this._w=h*u*d+f*p*y;break;default:console.warn("THREE.Quaternion: .setFromEuler() encountered an unknown order: "+o)}return e===!0&&this._onChangeCallback(),this}setFromAxisAngle(t,e){const n=e/2,r=Math.sin(n);return this._x=t.x*r,this._y=t.y*r,this._z=t.z*r,this._w=Math.cos(n),this._onChangeCallback(),this}setFromRotationMatrix(t){const e=t.elements,n=e[0],r=e[4],s=e[8],o=e[1],a=e[5],c=e[9],h=e[2],u=e[6],d=e[10],f=n+a+d;if(f>0){const p=.5/Math.sqrt(f+1);this._w=.25/p,this._x=(u-c)*p,this._y=(s-h)*p,this._z=(o-r)*p}else if(n>a&&n>d){const p=2*Math.sqrt(1+n-a-d);this._w=(u-c)/p,this._x=.25*p,this._y=(r+o)/p,this._z=(s+h)/p}else if(a>d){const p=2*Math.sqrt(1+a-n-d);this._w=(s-h)/p,this._x=(r+o)/p,this._y=.25*p,this._z=(c+u)/p}else{const p=2*Math.sqrt(1+d-n-a);this._w=(o-r)/p,this._x=(s+h)/p,this._y=(c+u)/p,this._z=.25*p}return this._onChangeCallback(),this}setFromUnitVectors(t,e){let n=t.dot(e)+1;return n<Number.EPSILON?(n=0,Math.abs(t.x)>Math.abs(t.z)?(this._x=-t.y,this._y=t.x,this._z=0,this._w=n):(this._x=0,this._y=-t.z,this._z=t.y,this._w=n)):(this._x=t.y*e.z-t.z*e.y,this._y=t.z*e.x-t.x*e.z,this._z=t.x*e.y-t.y*e.x,this._w=n),this.normalize()}angleTo(t){return 2*Math.acos(Math.abs(mn(this.dot(t),-1,1)))}rotateTowards(t,e){const n=this.angleTo(t);if(n===0)return this;const r=Math.min(1,e/n);return this.slerp(t,r),this}identity(){return this.set(0,0,0,1)}invert(){return this.conjugate()}conjugate(){return this._x*=-1,this._y*=-1,this._z*=-1,this._onChangeCallback(),this}dot(t){return this._x*t._x+this._y*t._y+this._z*t._z+this._w*t._w}lengthSq(){return this._x*this._x+this._y*this._y+this._z*this._z+this._w*this._w}length(){return Math.sqrt(this._x*this._x+this._y*this._y+this._z*this._z+this._w*this._w)}normalize(){let t=this.length();return t===0?(this._x=0,this._y=0,this._z=0,this._w=1):(t=1/t,this._x=this._x*t,this._y=this._y*t,this._z=this._z*t,this._w=this._w*t),this._onChangeCallback(),this}multiply(t){return this.multiplyQuaternions(this,t)}premultiply(t){return this.multiplyQuaternions(t,this)}multiplyQuaternions(t,e){const n=t._x,r=t._y,s=t._z,o=t._w,a=e._x,c=e._y,h=e._z,u=e._w;return this._x=n*u+o*a+r*h-s*c,this._y=r*u+o*c+s*a-n*h,this._z=s*u+o*h+n*c-r*a,this._w=o*u-n*a-r*c-s*h,this._onChangeCallback(),this}slerp(t,e){if(e===0)return this;if(e===1)return this.copy(t);const n=this._x,r=this._y,s=this._z,o=this._w;let a=o*t._w+n*t._x+r*t._y+s*t._z;if(a<0?(this._w=-t._w,this._x=-t._x,this._y=-t._y,this._z=-t._z,a=-a):this.copy(t),a>=1)return this._w=o,this._x=n,this._y=r,this._z=s,this;const c=1-a*a;if(c<=Number.EPSILON){const p=1-e;return this._w=p*o+e*this._w,this._x=p*n+e*this._x,this._y=p*r+e*this._y,this._z=p*s+e*this._z,this.normalize(),this}const h=Math.sqrt(c),u=Math.atan2(h,a),d=Math.sin((1-e)*u)/h,f=Math.sin(e*u)/h;return this._w=o*d+this._w*f,this._x=n*d+this._x*f,this._y=r*d+this._y*f,this._z=s*d+this._z*f,this._onChangeCallback(),this}slerpQuaternions(t,e,n){return this.copy(t).slerp(e,n)}random(){const t=Math.random(),e=Math.sqrt(1-t),n=Math.sqrt(t),r=2*Math.PI*Math.random(),s=2*Math.PI*Math.random();return this.set(e*Math.cos(r),n*Math.sin(s),n*Math.cos(s),e*Math.sin(r))}equals(t){return t._x===this._x&&t._y===this._y&&t._z===this._z&&t._w===this._w}fromArray(t,e=0){return this._x=t[e],this._y=t[e+1],this._z=t[e+2],this._w=t[e+3],this._onChangeCallback(),this}toArray(t=[],e=0){return t[e]=this._x,t[e+1]=this._y,t[e+2]=this._z,t[e+3]=this._w,t}fromBufferAttribute(t,e){return this._x=t.getX(e),this._y=t.getY(e),this._z=t.getZ(e),this._w=t.getW(e),this._onChangeCallback(),this}toJSON(){return this.toArray()}_onChange(t){return this._onChangeCallback=t,this}_onChangeCallback(){}*[Symbol.iterator](){yield this._x,yield this._y,yield this._z,yield this._w}}class k{constructor(t=0,e=0,n=0){k.prototype.isVector3=!0,this.x=t,this.y=e,this.z=n}set(t,e,n){return n===void 0&&(n=this.z),this.x=t,this.y=e,this.z=n,this}setScalar(t){return this.x=t,this.y=t,this.z=t,this}setX(t){return this.x=t,this}setY(t){return this.y=t,this}setZ(t){return this.z=t,this}setComponent(t,e){switch(t){case 0:this.x=e;break;case 1:this.y=e;break;case 2:this.z=e;break;default:throw new Error("index is out of range: "+t)}return this}getComponent(t){switch(t){case 0:return this.x;case 1:return this.y;case 2:return this.z;default:throw new Error("index is out of range: "+t)}}clone(){return new this.constructor(this.x,this.y,this.z)}copy(t){return this.x=t.x,this.y=t.y,this.z=t.z,this}add(t){return this.x+=t.x,this.y+=t.y,this.z+=t.z,this}addScalar(t){return this.x+=t,this.y+=t,this.z+=t,this}addVectors(t,e){return this.x=t.x+e.x,this.y=t.y+e.y,this.z=t.z+e.z,this}addScaledVector(t,e){return this.x+=t.x*e,this.y+=t.y*e,this.z+=t.z*e,this}sub(t){return this.x-=t.x,this.y-=t.y,this.z-=t.z,this}subScalar(t){return this.x-=t,this.y-=t,this.z-=t,this}subVectors(t,e){return this.x=t.x-e.x,this.y=t.y-e.y,this.z=t.z-e.z,this}multiply(t){return this.x*=t.x,this.y*=t.y,this.z*=t.z,this}multiplyScalar(t){return this.x*=t,this.y*=t,this.z*=t,this}multiplyVectors(t,e){return this.x=t.x*e.x,this.y=t.y*e.y,this.z=t.z*e.z,this}applyEuler(t){return this.applyQuaternion(zu.setFromEuler(t))}applyAxisAngle(t,e){return this.applyQuaternion(zu.setFromAxisAngle(t,e))}applyMatrix3(t){const e=this.x,n=this.y,r=this.z,s=t.elements;return this.x=s[0]*e+s[3]*n+s[6]*r,this.y=s[1]*e+s[4]*n+s[7]*r,this.z=s[2]*e+s[5]*n+s[8]*r,this}applyNormalMatrix(t){return this.applyMatrix3(t).normalize()}applyMatrix4(t){const e=this.x,n=this.y,r=this.z,s=t.elements,o=1/(s[3]*e+s[7]*n+s[11]*r+s[15]);return this.x=(s[0]*e+s[4]*n+s[8]*r+s[12])*o,this.y=(s[1]*e+s[5]*n+s[9]*r+s[13])*o,this.z=(s[2]*e+s[6]*n+s[10]*r+s[14])*o,this}applyQuaternion(t){const e=this.x,n=this.y,r=this.z,s=t.x,o=t.y,a=t.z,c=t.w,h=2*(o*r-a*n),u=2*(a*e-s*r),d=2*(s*n-o*e);return this.x=e+c*h+o*d-a*u,this.y=n+c*u+a*h-s*d,this.z=r+c*d+s*u-o*h,this}project(t){return this.applyMatrix4(t.matrixWorldInverse).applyMatrix4(t.projectionMatrix)}unproject(t){return this.applyMatrix4(t.projectionMatrixInverse).applyMatrix4(t.matrixWorld)}transformDirection(t){const e=this.x,n=this.y,r=this.z,s=t.elements;return this.x=s[0]*e+s[4]*n+s[8]*r,this.y=s[1]*e+s[5]*n+s[9]*r,this.z=s[2]*e+s[6]*n+s[10]*r,this.normalize()}divide(t){return this.x/=t.x,this.y/=t.y,this.z/=t.z,this}divideScalar(t){return this.multiplyScalar(1/t)}min(t){return this.x=Math.min(this.x,t.x),this.y=Math.min(this.y,t.y),this.z=Math.min(this.z,t.z),this}max(t){return this.x=Math.max(this.x,t.x),this.y=Math.max(this.y,t.y),this.z=Math.max(this.z,t.z),this}clamp(t,e){return this.x=Math.max(t.x,Math.min(e.x,this.x)),this.y=Math.max(t.y,Math.min(e.y,this.y)),this.z=Math.max(t.z,Math.min(e.z,this.z)),this}clampScalar(t,e){return this.x=Math.max(t,Math.min(e,this.x)),this.y=Math.max(t,Math.min(e,this.y)),this.z=Math.max(t,Math.min(e,this.z)),this}clampLength(t,e){const n=this.length();return this.divideScalar(n||1).multiplyScalar(Math.max(t,Math.min(e,n)))}floor(){return this.x=Math.floor(this.x),this.y=Math.floor(this.y),this.z=Math.floor(this.z),this}ceil(){return this.x=Math.ceil(this.x),this.y=Math.ceil(this.y),this.z=Math.ceil(this.z),this}round(){return this.x=Math.round(this.x),this.y=Math.round(this.y),this.z=Math.round(this.z),this}roundToZero(){return this.x=Math.trunc(this.x),this.y=Math.trunc(this.y),this.z=Math.trunc(this.z),this}negate(){return this.x=-this.x,this.y=-this.y,this.z=-this.z,this}dot(t){return this.x*t.x+this.y*t.y+this.z*t.z}lengthSq(){return this.x*this.x+this.y*this.y+this.z*this.z}length(){return Math.sqrt(this.x*this.x+this.y*this.y+this.z*this.z)}manhattanLength(){return Math.abs(this.x)+Math.abs(this.y)+Math.abs(this.z)}normalize(){return this.divideScalar(this.length()||1)}setLength(t){return this.normalize().multiplyScalar(t)}lerp(t,e){return this.x+=(t.x-this.x)*e,this.y+=(t.y-this.y)*e,this.z+=(t.z-this.z)*e,this}lerpVectors(t,e,n){return this.x=t.x+(e.x-t.x)*n,this.y=t.y+(e.y-t.y)*n,this.z=t.z+(e.z-t.z)*n,this}cross(t){return this.crossVectors(this,t)}crossVectors(t,e){const n=t.x,r=t.y,s=t.z,o=e.x,a=e.y,c=e.z;return this.x=r*c-s*a,this.y=s*o-n*c,this.z=n*a-r*o,this}projectOnVector(t){const e=t.lengthSq();if(e===0)return this.set(0,0,0);const n=t.dot(this)/e;return this.copy(t).multiplyScalar(n)}projectOnPlane(t){return Sc.copy(this).projectOnVector(t),this.sub(Sc)}reflect(t){return this.sub(Sc.copy(t).multiplyScalar(2*this.dot(t)))}angleTo(t){const e=Math.sqrt(this.lengthSq()*t.lengthSq());if(e===0)return Math.PI/2;const n=this.dot(t)/e;return Math.acos(mn(n,-1,1))}distanceTo(t){return Math.sqrt(this.distanceToSquared(t))}distanceToSquared(t){const e=this.x-t.x,n=this.y-t.y,r=this.z-t.z;return e*e+n*n+r*r}manhattanDistanceTo(t){return Math.abs(this.x-t.x)+Math.abs(this.y-t.y)+Math.abs(this.z-t.z)}setFromSpherical(t){return this.setFromSphericalCoords(t.radius,t.phi,t.theta)}setFromSphericalCoords(t,e,n){const r=Math.sin(e)*t;return this.x=r*Math.sin(n),this.y=Math.cos(e)*t,this.z=r*Math.cos(n),this}setFromCylindrical(t){return this.setFromCylindricalCoords(t.radius,t.theta,t.y)}setFromCylindricalCoords(t,e,n){return this.x=t*Math.sin(e),this.y=n,this.z=t*Math.cos(e),this}setFromMatrixPosition(t){const e=t.elements;return this.x=e[12],this.y=e[13],this.z=e[14],this}setFromMatrixScale(t){const e=this.setFromMatrixColumn(t,0).length(),n=this.setFromMatrixColumn(t,1).length(),r=this.setFromMatrixColumn(t,2).length();return this.x=e,this.y=n,this.z=r,this}setFromMatrixColumn(t,e){return this.fromArray(t.elements,e*4)}setFromMatrix3Column(t,e){return this.fromArray(t.elements,e*3)}setFromEuler(t){return this.x=t._x,this.y=t._y,this.z=t._z,this}setFromColor(t){return this.x=t.r,this.y=t.g,this.z=t.b,this}equals(t){return t.x===this.x&&t.y===this.y&&t.z===this.z}fromArray(t,e=0){return this.x=t[e],this.y=t[e+1],this.z=t[e+2],this}toArray(t=[],e=0){return t[e]=this.x,t[e+1]=this.y,t[e+2]=this.z,t}fromBufferAttribute(t,e){return this.x=t.getX(e),this.y=t.getY(e),this.z=t.getZ(e),this}random(){return this.x=Math.random(),this.y=Math.random(),this.z=Math.random(),this}randomDirection(){const t=(Math.random()-.5)*2,e=Math.random()*Math.PI*2,n=Math.sqrt(1-t**2);return this.x=n*Math.cos(e),this.y=n*Math.sin(e),this.z=t,this}*[Symbol.iterator](){yield this.x,yield this.y,yield this.z}}const Sc=new k,zu=new ur;class po{constructor(t=new k(1/0,1/0,1/0),e=new k(-1/0,-1/0,-1/0)){this.isBox3=!0,this.min=t,this.max=e}set(t,e){return this.min.copy(t),this.max.copy(e),this}setFromArray(t){this.makeEmpty();for(let e=0,n=t.length;e<n;e+=3)this.expandByPoint(Bn.fromArray(t,e));return this}setFromBufferAttribute(t){this.makeEmpty();for(let e=0,n=t.count;e<n;e++)this.expandByPoint(Bn.fromBufferAttribute(t,e));return this}setFromPoints(t){this.makeEmpty();for(let e=0,n=t.length;e<n;e++)this.expandByPoint(t[e]);return this}setFromCenterAndSize(t,e){const n=Bn.copy(e).multiplyScalar(.5);return this.min.copy(t).sub(n),this.max.copy(t).add(n),this}setFromObject(t,e=!1){return this.makeEmpty(),this.expandByObject(t,e)}clone(){return new this.constructor().copy(this)}copy(t){return this.min.copy(t.min),this.max.copy(t.max),this}makeEmpty(){return this.min.x=this.min.y=this.min.z=1/0,this.max.x=this.max.y=this.max.z=-1/0,this}isEmpty(){return this.max.x<this.min.x||this.max.y<this.min.y||this.max.z<this.min.z}getCenter(t){return this.isEmpty()?t.set(0,0,0):t.addVectors(this.min,this.max).multiplyScalar(.5)}getSize(t){return this.isEmpty()?t.set(0,0,0):t.subVectors(this.max,this.min)}expandByPoint(t){return this.min.min(t),this.max.max(t),this}expandByVector(t){return this.min.sub(t),this.max.add(t),this}expandByScalar(t){return this.min.addScalar(-t),this.max.addScalar(t),this}expandByObject(t,e=!1){t.updateWorldMatrix(!1,!1);const n=t.geometry;if(n!==void 0){const s=n.getAttribute("position");if(e===!0&&s!==void 0&&t.isInstancedMesh!==!0)for(let o=0,a=s.count;o<a;o++)t.isMesh===!0?t.getVertexPosition(o,Bn):Bn.fromBufferAttribute(s,o),Bn.applyMatrix4(t.matrixWorld),this.expandByPoint(Bn);else t.boundingBox!==void 0?(t.boundingBox===null&&t.computeBoundingBox(),No.copy(t.boundingBox)):(n.boundingBox===null&&n.computeBoundingBox(),No.copy(n.boundingBox)),No.applyMatrix4(t.matrixWorld),this.union(No)}const r=t.children;for(let s=0,o=r.length;s<o;s++)this.expandByObject(r[s],e);return this}containsPoint(t){return!(t.x<this.min.x||t.x>this.max.x||t.y<this.min.y||t.y>this.max.y||t.z<this.min.z||t.z>this.max.z)}containsBox(t){return this.min.x<=t.min.x&&t.max.x<=this.max.x&&this.min.y<=t.min.y&&t.max.y<=this.max.y&&this.min.z<=t.min.z&&t.max.z<=this.max.z}getParameter(t,e){return e.set((t.x-this.min.x)/(this.max.x-this.min.x),(t.y-this.min.y)/(this.max.y-this.min.y),(t.z-this.min.z)/(this.max.z-this.min.z))}intersectsBox(t){return!(t.max.x<this.min.x||t.min.x>this.max.x||t.max.y<this.min.y||t.min.y>this.max.y||t.max.z<this.min.z||t.min.z>this.max.z)}intersectsSphere(t){return this.clampPoint(t.center,Bn),Bn.distanceToSquared(t.center)<=t.radius*t.radius}intersectsPlane(t){let e,n;return t.normal.x>0?(e=t.normal.x*this.min.x,n=t.normal.x*this.max.x):(e=t.normal.x*this.max.x,n=t.normal.x*this.min.x),t.normal.y>0?(e+=t.normal.y*this.min.y,n+=t.normal.y*this.max.y):(e+=t.normal.y*this.max.y,n+=t.normal.y*this.min.y),t.normal.z>0?(e+=t.normal.z*this.min.z,n+=t.normal.z*this.max.z):(e+=t.normal.z*this.max.z,n+=t.normal.z*this.min.z),e<=-t.constant&&n>=-t.constant}intersectsTriangle(t){if(this.isEmpty())return!1;this.getCenter(Os),Uo.subVectors(this.max,Os),Cr.subVectors(t.a,Os),Ir.subVectors(t.b,Os),Pr.subVectors(t.c,Os),Ei.subVectors(Ir,Cr),xi.subVectors(Pr,Ir),$i.subVectors(Cr,Pr);let e=[0,-Ei.z,Ei.y,0,-xi.z,xi.y,0,-$i.z,$i.y,Ei.z,0,-Ei.x,xi.z,0,-xi.x,$i.z,0,-$i.x,-Ei.y,Ei.x,0,-xi.y,xi.x,0,-$i.y,$i.x,0];return!Tc(e,Cr,Ir,Pr,Uo)||(e=[1,0,0,0,1,0,0,0,1],!Tc(e,Cr,Ir,Pr,Uo))?!1:(Oo.crossVectors(Ei,xi),e=[Oo.x,Oo.y,Oo.z],Tc(e,Cr,Ir,Pr,Uo))}clampPoint(t,e){return e.copy(t).clamp(this.min,this.max)}distanceToPoint(t){return this.clampPoint(t,Bn).distanceTo(t)}getBoundingSphere(t){return this.isEmpty()?t.makeEmpty():(this.getCenter(t.center),t.radius=this.getSize(Bn).length()*.5),t}intersect(t){return this.min.max(t.min),this.max.min(t.max),this.isEmpty()&&this.makeEmpty(),this}union(t){return this.min.min(t.min),this.max.max(t.max),this}applyMatrix4(t){return this.isEmpty()?this:(ei[0].set(this.min.x,this.min.y,this.min.z).applyMatrix4(t),ei[1].set(this.min.x,this.min.y,this.max.z).applyMatrix4(t),ei[2].set(this.min.x,this.max.y,this.min.z).applyMatrix4(t),ei[3].set(this.min.x,this.max.y,this.max.z).applyMatrix4(t),ei[4].set(this.max.x,this.min.y,this.min.z).applyMatrix4(t),ei[5].set(this.max.x,this.min.y,this.max.z).applyMatrix4(t),ei[6].set(this.max.x,this.max.y,this.min.z).applyMatrix4(t),ei[7].set(this.max.x,this.max.y,this.max.z).applyMatrix4(t),this.setFromPoints(ei),this)}translate(t){return this.min.add(t),this.max.add(t),this}equals(t){return t.min.equals(this.min)&&t.max.equals(this.max)}}const ei=[new k,new k,new k,new k,new k,new k,new k,new k],Bn=new k,No=new po,Cr=new k,Ir=new k,Pr=new k,Ei=new k,xi=new k,$i=new k,Os=new k,Uo=new k,Oo=new k,Ki=new k;function Tc(i,t,e,n,r){for(let s=0,o=i.length-3;s<=o;s+=3){Ki.fromArray(i,s);const a=r.x*Math.abs(Ki.x)+r.y*Math.abs(Ki.y)+r.z*Math.abs(Ki.z),c=t.dot(Ki),h=e.dot(Ki),u=n.dot(Ki);if(Math.max(-Math.max(c,h,u),Math.min(c,h,u))>a)return!1}return!0}const I_=new po,Fs=new k,Mc=new k;class Bl{constructor(t=new k,e=-1){this.isSphere=!0,this.center=t,this.radius=e}set(t,e){return this.center.copy(t),this.radius=e,this}setFromPoints(t,e){const n=this.center;e!==void 0?n.copy(e):I_.setFromPoints(t).getCenter(n);let r=0;for(let s=0,o=t.length;s<o;s++)r=Math.max(r,n.distanceToSquared(t[s]));return this.radius=Math.sqrt(r),this}copy(t){return this.center.copy(t.center),this.radius=t.radius,this}isEmpty(){return this.radius<0}makeEmpty(){return this.center.set(0,0,0),this.radius=-1,this}containsPoint(t){return t.distanceToSquared(this.center)<=this.radius*this.radius}distanceToPoint(t){return t.distanceTo(this.center)-this.radius}intersectsSphere(t){const e=this.radius+t.radius;return t.center.distanceToSquared(this.center)<=e*e}intersectsBox(t){return t.intersectsSphere(this)}intersectsPlane(t){return Math.abs(t.distanceToPoint(this.center))<=this.radius}clampPoint(t,e){const n=this.center.distanceToSquared(t);return e.copy(t),n>this.radius*this.radius&&(e.sub(this.center).normalize(),e.multiplyScalar(this.radius).add(this.center)),e}getBoundingBox(t){return this.isEmpty()?(t.makeEmpty(),t):(t.set(this.center,this.center),t.expandByScalar(this.radius),t)}applyMatrix4(t){return this.center.applyMatrix4(t),this.radius=this.radius*t.getMaxScaleOnAxis(),this}translate(t){return this.center.add(t),this}expandByPoint(t){if(this.isEmpty())return this.center.copy(t),this.radius=0,this;Fs.subVectors(t,this.center);const e=Fs.lengthSq();if(e>this.radius*this.radius){const n=Math.sqrt(e),r=(n-this.radius)*.5;this.center.addScaledVector(Fs,r/n),this.radius+=r}return this}union(t){return t.isEmpty()?this:this.isEmpty()?(this.copy(t),this):(this.center.equals(t.center)===!0?this.radius=Math.max(this.radius,t.radius):(Mc.subVectors(t.center,this.center).setLength(t.radius),this.expandByPoint(Fs.copy(t.center).add(Mc)),this.expandByPoint(Fs.copy(t.center).sub(Mc))),this)}equals(t){return t.center.equals(this.center)&&t.radius===this.radius}clone(){return new this.constructor().copy(this)}}const ni=new k,wc=new k,Fo=new k,Si=new k,bc=new k,Vo=new k,Ac=new k;class kl{constructor(t=new k,e=new k(0,0,-1)){this.origin=t,this.direction=e}set(t,e){return this.origin.copy(t),this.direction.copy(e),this}copy(t){return this.origin.copy(t.origin),this.direction.copy(t.direction),this}at(t,e){return e.copy(this.origin).addScaledVector(this.direction,t)}lookAt(t){return this.direction.copy(t).sub(this.origin).normalize(),this}recast(t){return this.origin.copy(this.at(t,ni)),this}closestPointToPoint(t,e){e.subVectors(t,this.origin);const n=e.dot(this.direction);return n<0?e.copy(this.origin):e.copy(this.origin).addScaledVector(this.direction,n)}distanceToPoint(t){return Math.sqrt(this.distanceSqToPoint(t))}distanceSqToPoint(t){const e=ni.subVectors(t,this.origin).dot(this.direction);return e<0?this.origin.distanceToSquared(t):(ni.copy(this.origin).addScaledVector(this.direction,e),ni.distanceToSquared(t))}distanceSqToSegment(t,e,n,r){wc.copy(t).add(e).multiplyScalar(.5),Fo.copy(e).sub(t).normalize(),Si.copy(this.origin).sub(wc);const s=t.distanceTo(e)*.5,o=-this.direction.dot(Fo),a=Si.dot(this.direction),c=-Si.dot(Fo),h=Si.lengthSq(),u=Math.abs(1-o*o);let d,f,p,y;if(u>0)if(d=o*c-a,f=o*a-c,y=s*u,d>=0)if(f>=-y)if(f<=y){const E=1/u;d*=E,f*=E,p=d*(d+o*f+2*a)+f*(o*d+f+2*c)+h}else f=s,d=Math.max(0,-(o*f+a)),p=-d*d+f*(f+2*c)+h;else f=-s,d=Math.max(0,-(o*f+a)),p=-d*d+f*(f+2*c)+h;else f<=-y?(d=Math.max(0,-(-o*s+a)),f=d>0?-s:Math.min(Math.max(-s,-c),s),p=-d*d+f*(f+2*c)+h):f<=y?(d=0,f=Math.min(Math.max(-s,-c),s),p=f*(f+2*c)+h):(d=Math.max(0,-(o*s+a)),f=d>0?s:Math.min(Math.max(-s,-c),s),p=-d*d+f*(f+2*c)+h);else f=o>0?-s:s,d=Math.max(0,-(o*f+a)),p=-d*d+f*(f+2*c)+h;return n&&n.copy(this.origin).addScaledVector(this.direction,d),r&&r.copy(wc).addScaledVector(Fo,f),p}intersectSphere(t,e){ni.subVectors(t.center,this.origin);const n=ni.dot(this.direction),r=ni.dot(ni)-n*n,s=t.radius*t.radius;if(r>s)return null;const o=Math.sqrt(s-r),a=n-o,c=n+o;return c<0?null:a<0?this.at(c,e):this.at(a,e)}intersectsSphere(t){return this.distanceSqToPoint(t.center)<=t.radius*t.radius}distanceToPlane(t){const e=t.normal.dot(this.direction);if(e===0)return t.distanceToPoint(this.origin)===0?0:null;const n=-(this.origin.dot(t.normal)+t.constant)/e;return n>=0?n:null}intersectPlane(t,e){const n=this.distanceToPlane(t);return n===null?null:this.at(n,e)}intersectsPlane(t){const e=t.distanceToPoint(this.origin);return e===0||t.normal.dot(this.direction)*e<0}intersectBox(t,e){let n,r,s,o,a,c;const h=1/this.direction.x,u=1/this.direction.y,d=1/this.direction.z,f=this.origin;return h>=0?(n=(t.min.x-f.x)*h,r=(t.max.x-f.x)*h):(n=(t.max.x-f.x)*h,r=(t.min.x-f.x)*h),u>=0?(s=(t.min.y-f.y)*u,o=(t.max.y-f.y)*u):(s=(t.max.y-f.y)*u,o=(t.min.y-f.y)*u),n>o||s>r||((s>n||isNaN(n))&&(n=s),(o<r||isNaN(r))&&(r=o),d>=0?(a=(t.min.z-f.z)*d,c=(t.max.z-f.z)*d):(a=(t.max.z-f.z)*d,c=(t.min.z-f.z)*d),n>c||a>r)||((a>n||n!==n)&&(n=a),(c<r||r!==r)&&(r=c),r<0)?null:this.at(n>=0?n:r,e)}intersectsBox(t){return this.intersectBox(t,ni)!==null}intersectTriangle(t,e,n,r,s){bc.subVectors(e,t),Vo.subVectors(n,t),Ac.crossVectors(bc,Vo);let o=this.direction.dot(Ac),a;if(o>0){if(r)return null;a=1}else if(o<0)a=-1,o=-o;else return null;Si.subVectors(this.origin,t);const c=a*this.direction.dot(Vo.crossVectors(Si,Vo));if(c<0)return null;const h=a*this.direction.dot(bc.cross(Si));if(h<0||c+h>o)return null;const u=-a*Si.dot(Ac);return u<0?null:this.at(u/o,s)}applyMatrix4(t){return this.origin.applyMatrix4(t),this.direction.transformDirection(t),this}equals(t){return t.origin.equals(this.origin)&&t.direction.equals(this.direction)}clone(){return new this.constructor().copy(this)}}class Be{constructor(t,e,n,r,s,o,a,c,h,u,d,f,p,y,E,_){Be.prototype.isMatrix4=!0,this.elements=[1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1],t!==void 0&&this.set(t,e,n,r,s,o,a,c,h,u,d,f,p,y,E,_)}set(t,e,n,r,s,o,a,c,h,u,d,f,p,y,E,_){const m=this.elements;return m[0]=t,m[4]=e,m[8]=n,m[12]=r,m[1]=s,m[5]=o,m[9]=a,m[13]=c,m[2]=h,m[6]=u,m[10]=d,m[14]=f,m[3]=p,m[7]=y,m[11]=E,m[15]=_,this}identity(){return this.set(1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1),this}clone(){return new Be().fromArray(this.elements)}copy(t){const e=this.elements,n=t.elements;return e[0]=n[0],e[1]=n[1],e[2]=n[2],e[3]=n[3],e[4]=n[4],e[5]=n[5],e[6]=n[6],e[7]=n[7],e[8]=n[8],e[9]=n[9],e[10]=n[10],e[11]=n[11],e[12]=n[12],e[13]=n[13],e[14]=n[14],e[15]=n[15],this}copyPosition(t){const e=this.elements,n=t.elements;return e[12]=n[12],e[13]=n[13],e[14]=n[14],this}setFromMatrix3(t){const e=t.elements;return this.set(e[0],e[3],e[6],0,e[1],e[4],e[7],0,e[2],e[5],e[8],0,0,0,0,1),this}extractBasis(t,e,n){return t.setFromMatrixColumn(this,0),e.setFromMatrixColumn(this,1),n.setFromMatrixColumn(this,2),this}makeBasis(t,e,n){return this.set(t.x,e.x,n.x,0,t.y,e.y,n.y,0,t.z,e.z,n.z,0,0,0,0,1),this}extractRotation(t){const e=this.elements,n=t.elements,r=1/Dr.setFromMatrixColumn(t,0).length(),s=1/Dr.setFromMatrixColumn(t,1).length(),o=1/Dr.setFromMatrixColumn(t,2).length();return e[0]=n[0]*r,e[1]=n[1]*r,e[2]=n[2]*r,e[3]=0,e[4]=n[4]*s,e[5]=n[5]*s,e[6]=n[6]*s,e[7]=0,e[8]=n[8]*o,e[9]=n[9]*o,e[10]=n[10]*o,e[11]=0,e[12]=0,e[13]=0,e[14]=0,e[15]=1,this}makeRotationFromEuler(t){const e=this.elements,n=t.x,r=t.y,s=t.z,o=Math.cos(n),a=Math.sin(n),c=Math.cos(r),h=Math.sin(r),u=Math.cos(s),d=Math.sin(s);if(t.order==="XYZ"){const f=o*u,p=o*d,y=a*u,E=a*d;e[0]=c*u,e[4]=-c*d,e[8]=h,e[1]=p+y*h,e[5]=f-E*h,e[9]=-a*c,e[2]=E-f*h,e[6]=y+p*h,e[10]=o*c}else if(t.order==="YXZ"){const f=c*u,p=c*d,y=h*u,E=h*d;e[0]=f+E*a,e[4]=y*a-p,e[8]=o*h,e[1]=o*d,e[5]=o*u,e[9]=-a,e[2]=p*a-y,e[6]=E+f*a,e[10]=o*c}else if(t.order==="ZXY"){const f=c*u,p=c*d,y=h*u,E=h*d;e[0]=f-E*a,e[4]=-o*d,e[8]=y+p*a,e[1]=p+y*a,e[5]=o*u,e[9]=E-f*a,e[2]=-o*h,e[6]=a,e[10]=o*c}else if(t.order==="ZYX"){const f=o*u,p=o*d,y=a*u,E=a*d;e[0]=c*u,e[4]=y*h-p,e[8]=f*h+E,e[1]=c*d,e[5]=E*h+f,e[9]=p*h-y,e[2]=-h,e[6]=a*c,e[10]=o*c}else if(t.order==="YZX"){const f=o*c,p=o*h,y=a*c,E=a*h;e[0]=c*u,e[4]=E-f*d,e[8]=y*d+p,e[1]=d,e[5]=o*u,e[9]=-a*u,e[2]=-h*u,e[6]=p*d+y,e[10]=f-E*d}else if(t.order==="XZY"){const f=o*c,p=o*h,y=a*c,E=a*h;e[0]=c*u,e[4]=-d,e[8]=h*u,e[1]=f*d+E,e[5]=o*u,e[9]=p*d-y,e[2]=y*d-p,e[6]=a*u,e[10]=E*d+f}return e[3]=0,e[7]=0,e[11]=0,e[12]=0,e[13]=0,e[14]=0,e[15]=1,this}makeRotationFromQuaternion(t){return this.compose(P_,t,D_)}lookAt(t,e,n){const r=this.elements;return Mn.subVectors(t,e),Mn.lengthSq()===0&&(Mn.z=1),Mn.normalize(),Ti.crossVectors(n,Mn),Ti.lengthSq()===0&&(Math.abs(n.z)===1?Mn.x+=1e-4:Mn.z+=1e-4,Mn.normalize(),Ti.crossVectors(n,Mn)),Ti.normalize(),Bo.crossVectors(Mn,Ti),r[0]=Ti.x,r[4]=Bo.x,r[8]=Mn.x,r[1]=Ti.y,r[5]=Bo.y,r[9]=Mn.y,r[2]=Ti.z,r[6]=Bo.z,r[10]=Mn.z,this}multiply(t){return this.multiplyMatrices(this,t)}premultiply(t){return this.multiplyMatrices(t,this)}multiplyMatrices(t,e){const n=t.elements,r=e.elements,s=this.elements,o=n[0],a=n[4],c=n[8],h=n[12],u=n[1],d=n[5],f=n[9],p=n[13],y=n[2],E=n[6],_=n[10],m=n[14],C=n[3],w=n[7],b=n[11],N=n[15],U=r[0],T=r[4],M=r[8],v=r[12],S=r[1],P=r[5],L=r[9],R=r[13],V=r[2],G=r[6],K=r[10],Z=r[14],J=r[3],et=r[7],tt=r[11],pt=r[15];return s[0]=o*U+a*S+c*V+h*J,s[4]=o*T+a*P+c*G+h*et,s[8]=o*M+a*L+c*K+h*tt,s[12]=o*v+a*R+c*Z+h*pt,s[1]=u*U+d*S+f*V+p*J,s[5]=u*T+d*P+f*G+p*et,s[9]=u*M+d*L+f*K+p*tt,s[13]=u*v+d*R+f*Z+p*pt,s[2]=y*U+E*S+_*V+m*J,s[6]=y*T+E*P+_*G+m*et,s[10]=y*M+E*L+_*K+m*tt,s[14]=y*v+E*R+_*Z+m*pt,s[3]=C*U+w*S+b*V+N*J,s[7]=C*T+w*P+b*G+N*et,s[11]=C*M+w*L+b*K+N*tt,s[15]=C*v+w*R+b*Z+N*pt,this}multiplyScalar(t){const e=this.elements;return e[0]*=t,e[4]*=t,e[8]*=t,e[12]*=t,e[1]*=t,e[5]*=t,e[9]*=t,e[13]*=t,e[2]*=t,e[6]*=t,e[10]*=t,e[14]*=t,e[3]*=t,e[7]*=t,e[11]*=t,e[15]*=t,this}determinant(){const t=this.elements,e=t[0],n=t[4],r=t[8],s=t[12],o=t[1],a=t[5],c=t[9],h=t[13],u=t[2],d=t[6],f=t[10],p=t[14],y=t[3],E=t[7],_=t[11],m=t[15];return y*(+s*c*d-r*h*d-s*a*f+n*h*f+r*a*p-n*c*p)+E*(+e*c*p-e*h*f+s*o*f-r*o*p+r*h*u-s*c*u)+_*(+e*h*d-e*a*p-s*o*d+n*o*p+s*a*u-n*h*u)+m*(-r*a*u-e*c*d+e*a*f+r*o*d-n*o*f+n*c*u)}transpose(){const t=this.elements;let e;return e=t[1],t[1]=t[4],t[4]=e,e=t[2],t[2]=t[8],t[8]=e,e=t[6],t[6]=t[9],t[9]=e,e=t[3],t[3]=t[12],t[12]=e,e=t[7],t[7]=t[13],t[13]=e,e=t[11],t[11]=t[14],t[14]=e,this}setPosition(t,e,n){const r=this.elements;return t.isVector3?(r[12]=t.x,r[13]=t.y,r[14]=t.z):(r[12]=t,r[13]=e,r[14]=n),this}invert(){const t=this.elements,e=t[0],n=t[1],r=t[2],s=t[3],o=t[4],a=t[5],c=t[6],h=t[7],u=t[8],d=t[9],f=t[10],p=t[11],y=t[12],E=t[13],_=t[14],m=t[15],C=d*_*h-E*f*h+E*c*p-a*_*p-d*c*m+a*f*m,w=y*f*h-u*_*h-y*c*p+o*_*p+u*c*m-o*f*m,b=u*E*h-y*d*h+y*a*p-o*E*p-u*a*m+o*d*m,N=y*d*c-u*E*c-y*a*f+o*E*f+u*a*_-o*d*_,U=e*C+n*w+r*b+s*N;if(U===0)return this.set(0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0);const T=1/U;return t[0]=C*T,t[1]=(E*f*s-d*_*s-E*r*p+n*_*p+d*r*m-n*f*m)*T,t[2]=(a*_*s-E*c*s+E*r*h-n*_*h-a*r*m+n*c*m)*T,t[3]=(d*c*s-a*f*s-d*r*h+n*f*h+a*r*p-n*c*p)*T,t[4]=w*T,t[5]=(u*_*s-y*f*s+y*r*p-e*_*p-u*r*m+e*f*m)*T,t[6]=(y*c*s-o*_*s-y*r*h+e*_*h+o*r*m-e*c*m)*T,t[7]=(o*f*s-u*c*s+u*r*h-e*f*h-o*r*p+e*c*p)*T,t[8]=b*T,t[9]=(y*d*s-u*E*s-y*n*p+e*E*p+u*n*m-e*d*m)*T,t[10]=(o*E*s-y*a*s+y*n*h-e*E*h-o*n*m+e*a*m)*T,t[11]=(u*a*s-o*d*s-u*n*h+e*d*h+o*n*p-e*a*p)*T,t[12]=N*T,t[13]=(u*E*r-y*d*r+y*n*f-e*E*f-u*n*_+e*d*_)*T,t[14]=(y*a*r-o*E*r-y*n*c+e*E*c+o*n*_-e*a*_)*T,t[15]=(o*d*r-u*a*r+u*n*c-e*d*c-o*n*f+e*a*f)*T,this}scale(t){const e=this.elements,n=t.x,r=t.y,s=t.z;return e[0]*=n,e[4]*=r,e[8]*=s,e[1]*=n,e[5]*=r,e[9]*=s,e[2]*=n,e[6]*=r,e[10]*=s,e[3]*=n,e[7]*=r,e[11]*=s,this}getMaxScaleOnAxis(){const t=this.elements,e=t[0]*t[0]+t[1]*t[1]+t[2]*t[2],n=t[4]*t[4]+t[5]*t[5]+t[6]*t[6],r=t[8]*t[8]+t[9]*t[9]+t[10]*t[10];return Math.sqrt(Math.max(e,n,r))}makeTranslation(t,e,n){return t.isVector3?this.set(1,0,0,t.x,0,1,0,t.y,0,0,1,t.z,0,0,0,1):this.set(1,0,0,t,0,1,0,e,0,0,1,n,0,0,0,1),this}makeRotationX(t){const e=Math.cos(t),n=Math.sin(t);return this.set(1,0,0,0,0,e,-n,0,0,n,e,0,0,0,0,1),this}makeRotationY(t){const e=Math.cos(t),n=Math.sin(t);return this.set(e,0,n,0,0,1,0,0,-n,0,e,0,0,0,0,1),this}makeRotationZ(t){const e=Math.cos(t),n=Math.sin(t);return this.set(e,-n,0,0,n,e,0,0,0,0,1,0,0,0,0,1),this}makeRotationAxis(t,e){const n=Math.cos(e),r=Math.sin(e),s=1-n,o=t.x,a=t.y,c=t.z,h=s*o,u=s*a;return this.set(h*o+n,h*a-r*c,h*c+r*a,0,h*a+r*c,u*a+n,u*c-r*o,0,h*c-r*a,u*c+r*o,s*c*c+n,0,0,0,0,1),this}makeScale(t,e,n){return this.set(t,0,0,0,0,e,0,0,0,0,n,0,0,0,0,1),this}makeShear(t,e,n,r,s,o){return this.set(1,n,s,0,t,1,o,0,e,r,1,0,0,0,0,1),this}compose(t,e,n){const r=this.elements,s=e._x,o=e._y,a=e._z,c=e._w,h=s+s,u=o+o,d=a+a,f=s*h,p=s*u,y=s*d,E=o*u,_=o*d,m=a*d,C=c*h,w=c*u,b=c*d,N=n.x,U=n.y,T=n.z;return r[0]=(1-(E+m))*N,r[1]=(p+b)*N,r[2]=(y-w)*N,r[3]=0,r[4]=(p-b)*U,r[5]=(1-(f+m))*U,r[6]=(_+C)*U,r[7]=0,r[8]=(y+w)*T,r[9]=(_-C)*T,r[10]=(1-(f+E))*T,r[11]=0,r[12]=t.x,r[13]=t.y,r[14]=t.z,r[15]=1,this}decompose(t,e,n){const r=this.elements;let s=Dr.set(r[0],r[1],r[2]).length();const o=Dr.set(r[4],r[5],r[6]).length(),a=Dr.set(r[8],r[9],r[10]).length();this.determinant()<0&&(s=-s),t.x=r[12],t.y=r[13],t.z=r[14],kn.copy(this);const h=1/s,u=1/o,d=1/a;return kn.elements[0]*=h,kn.elements[1]*=h,kn.elements[2]*=h,kn.elements[4]*=u,kn.elements[5]*=u,kn.elements[6]*=u,kn.elements[8]*=d,kn.elements[9]*=d,kn.elements[10]*=d,e.setFromRotationMatrix(kn),n.x=s,n.y=o,n.z=a,this}makePerspective(t,e,n,r,s,o,a=li){const c=this.elements,h=2*s/(e-t),u=2*s/(n-r),d=(e+t)/(e-t),f=(n+r)/(n-r);let p,y;if(a===li)p=-(o+s)/(o-s),y=-2*o*s/(o-s);else if(a===wa)p=-o/(o-s),y=-o*s/(o-s);else throw new Error("THREE.Matrix4.makePerspective(): Invalid coordinate system: "+a);return c[0]=h,c[4]=0,c[8]=d,c[12]=0,c[1]=0,c[5]=u,c[9]=f,c[13]=0,c[2]=0,c[6]=0,c[10]=p,c[14]=y,c[3]=0,c[7]=0,c[11]=-1,c[15]=0,this}makeOrthographic(t,e,n,r,s,o,a=li){const c=this.elements,h=1/(e-t),u=1/(n-r),d=1/(o-s),f=(e+t)*h,p=(n+r)*u;let y,E;if(a===li)y=(o+s)*d,E=-2*d;else if(a===wa)y=s*d,E=-1*d;else throw new Error("THREE.Matrix4.makeOrthographic(): Invalid coordinate system: "+a);return c[0]=2*h,c[4]=0,c[8]=0,c[12]=-f,c[1]=0,c[5]=2*u,c[9]=0,c[13]=-p,c[2]=0,c[6]=0,c[10]=E,c[14]=-y,c[3]=0,c[7]=0,c[11]=0,c[15]=1,this}equals(t){const e=this.elements,n=t.elements;for(let r=0;r<16;r++)if(e[r]!==n[r])return!1;return!0}fromArray(t,e=0){for(let n=0;n<16;n++)this.elements[n]=t[n+e];return this}toArray(t=[],e=0){const n=this.elements;return t[e]=n[0],t[e+1]=n[1],t[e+2]=n[2],t[e+3]=n[3],t[e+4]=n[4],t[e+5]=n[5],t[e+6]=n[6],t[e+7]=n[7],t[e+8]=n[8],t[e+9]=n[9],t[e+10]=n[10],t[e+11]=n[11],t[e+12]=n[12],t[e+13]=n[13],t[e+14]=n[14],t[e+15]=n[15],t}}const Dr=new k,kn=new Be,P_=new k(0,0,0),D_=new k(1,1,1),Ti=new k,Bo=new k,Mn=new k,Gu=new Be,Wu=new ur;class qa{constructor(t=0,e=0,n=0,r=qa.DEFAULT_ORDER){this.isEuler=!0,this._x=t,this._y=e,this._z=n,this._order=r}get x(){return this._x}set x(t){this._x=t,this._onChangeCallback()}get y(){return this._y}set y(t){this._y=t,this._onChangeCallback()}get z(){return this._z}set z(t){this._z=t,this._onChangeCallback()}get order(){return this._order}set order(t){this._order=t,this._onChangeCallback()}set(t,e,n,r=this._order){return this._x=t,this._y=e,this._z=n,this._order=r,this._onChangeCallback(),this}clone(){return new this.constructor(this._x,this._y,this._z,this._order)}copy(t){return this._x=t._x,this._y=t._y,this._z=t._z,this._order=t._order,this._onChangeCallback(),this}setFromRotationMatrix(t,e=this._order,n=!0){const r=t.elements,s=r[0],o=r[4],a=r[8],c=r[1],h=r[5],u=r[9],d=r[2],f=r[6],p=r[10];switch(e){case"XYZ":this._y=Math.asin(mn(a,-1,1)),Math.abs(a)<.9999999?(this._x=Math.atan2(-u,p),this._z=Math.atan2(-o,s)):(this._x=Math.atan2(f,h),this._z=0);break;case"YXZ":this._x=Math.asin(-mn(u,-1,1)),Math.abs(u)<.9999999?(this._y=Math.atan2(a,p),this._z=Math.atan2(c,h)):(this._y=Math.atan2(-d,s),this._z=0);break;case"ZXY":this._x=Math.asin(mn(f,-1,1)),Math.abs(f)<.9999999?(this._y=Math.atan2(-d,p),this._z=Math.atan2(-o,h)):(this._y=0,this._z=Math.atan2(c,s));break;case"ZYX":this._y=Math.asin(-mn(d,-1,1)),Math.abs(d)<.9999999?(this._x=Math.atan2(f,p),this._z=Math.atan2(c,s)):(this._x=0,this._z=Math.atan2(-o,h));break;case"YZX":this._z=Math.asin(mn(c,-1,1)),Math.abs(c)<.9999999?(this._x=Math.atan2(-u,h),this._y=Math.atan2(-d,s)):(this._x=0,this._y=Math.atan2(a,p));break;case"XZY":this._z=Math.asin(-mn(o,-1,1)),Math.abs(o)<.9999999?(this._x=Math.atan2(f,h),this._y=Math.atan2(a,s)):(this._x=Math.atan2(-u,p),this._y=0);break;default:console.warn("THREE.Euler: .setFromRotationMatrix() encountered an unknown order: "+e)}return this._order=e,n===!0&&this._onChangeCallback(),this}setFromQuaternion(t,e,n){return Gu.makeRotationFromQuaternion(t),this.setFromRotationMatrix(Gu,e,n)}setFromVector3(t,e=this._order){return this.set(t.x,t.y,t.z,e)}reorder(t){return Wu.setFromEuler(this),this.setFromQuaternion(Wu,t)}equals(t){return t._x===this._x&&t._y===this._y&&t._z===this._z&&t._order===this._order}fromArray(t){return this._x=t[0],this._y=t[1],this._z=t[2],t[3]!==void 0&&(this._order=t[3]),this._onChangeCallback(),this}toArray(t=[],e=0){return t[e]=this._x,t[e+1]=this._y,t[e+2]=this._z,t[e+3]=this._order,t}_onChange(t){return this._onChangeCallback=t,this}_onChangeCallback(){}*[Symbol.iterator](){yield this._x,yield this._y,yield this._z,yield this._order}}qa.DEFAULT_ORDER="XYZ";class Hl{constructor(){this.mask=1}set(t){this.mask=(1<<t|0)>>>0}enable(t){this.mask|=1<<t|0}enableAll(){this.mask=-1}toggle(t){this.mask^=1<<t|0}disable(t){this.mask&=~(1<<t|0)}disableAll(){this.mask=0}test(t){return(this.mask&t.mask)!==0}isEnabled(t){return(this.mask&(1<<t|0))!==0}}let L_=0;const qu=new k,Lr=new ur,ii=new Be,ko=new k,Vs=new k,N_=new k,U_=new ur,Xu=new k(1,0,0),ju=new k(0,1,0),Yu=new k(0,0,1),O_={type:"added"},F_={type:"removed"};class je extends gr{constructor(){super(),this.isObject3D=!0,Object.defineProperty(this,"id",{value:L_++}),this.uuid=Ui(),this.name="",this.type="Object3D",this.parent=null,this.children=[],this.up=je.DEFAULT_UP.clone();const t=new k,e=new qa,n=new ur,r=new k(1,1,1);function s(){n.setFromEuler(e,!1)}function o(){e.setFromQuaternion(n,void 0,!1)}e._onChange(s),n._onChange(o),Object.defineProperties(this,{position:{configurable:!0,enumerable:!0,value:t},rotation:{configurable:!0,enumerable:!0,value:e},quaternion:{configurable:!0,enumerable:!0,value:n},scale:{configurable:!0,enumerable:!0,value:r},modelViewMatrix:{value:new Be},normalMatrix:{value:new ae}}),this.matrix=new Be,this.matrixWorld=new Be,this.matrixAutoUpdate=je.DEFAULT_MATRIX_AUTO_UPDATE,this.matrixWorldAutoUpdate=je.DEFAULT_MATRIX_WORLD_AUTO_UPDATE,this.matrixWorldNeedsUpdate=!1,this.layers=new Hl,this.visible=!0,this.castShadow=!1,this.receiveShadow=!1,this.frustumCulled=!0,this.renderOrder=0,this.animations=[],this.userData={}}onBeforeShadow(){}onAfterShadow(){}onBeforeRender(){}onAfterRender(){}applyMatrix4(t){this.matrixAutoUpdate&&this.updateMatrix(),this.matrix.premultiply(t),this.matrix.decompose(this.position,this.quaternion,this.scale)}applyQuaternion(t){return this.quaternion.premultiply(t),this}setRotationFromAxisAngle(t,e){this.quaternion.setFromAxisAngle(t,e)}setRotationFromEuler(t){this.quaternion.setFromEuler(t,!0)}setRotationFromMatrix(t){this.quaternion.setFromRotationMatrix(t)}setRotationFromQuaternion(t){this.quaternion.copy(t)}rotateOnAxis(t,e){return Lr.setFromAxisAngle(t,e),this.quaternion.multiply(Lr),this}rotateOnWorldAxis(t,e){return Lr.setFromAxisAngle(t,e),this.quaternion.premultiply(Lr),this}rotateX(t){return this.rotateOnAxis(Xu,t)}rotateY(t){return this.rotateOnAxis(ju,t)}rotateZ(t){return this.rotateOnAxis(Yu,t)}translateOnAxis(t,e){return qu.copy(t).applyQuaternion(this.quaternion),this.position.add(qu.multiplyScalar(e)),this}translateX(t){return this.translateOnAxis(Xu,t)}translateY(t){return this.translateOnAxis(ju,t)}translateZ(t){return this.translateOnAxis(Yu,t)}localToWorld(t){return this.updateWorldMatrix(!0,!1),t.applyMatrix4(this.matrixWorld)}worldToLocal(t){return this.updateWorldMatrix(!0,!1),t.applyMatrix4(ii.copy(this.matrixWorld).invert())}lookAt(t,e,n){t.isVector3?ko.copy(t):ko.set(t,e,n);const r=this.parent;this.updateWorldMatrix(!0,!1),Vs.setFromMatrixPosition(this.matrixWorld),this.isCamera||this.isLight?ii.lookAt(Vs,ko,this.up):ii.lookAt(ko,Vs,this.up),this.quaternion.setFromRotationMatrix(ii),r&&(ii.extractRotation(r.matrixWorld),Lr.setFromRotationMatrix(ii),this.quaternion.premultiply(Lr.invert()))}add(t){if(arguments.length>1){for(let e=0;e<arguments.length;e++)this.add(arguments[e]);return this}return t===this?(console.error("THREE.Object3D.add: object can't be added as a child of itself.",t),this):(t&&t.isObject3D?(t.parent!==null&&t.parent.remove(t),t.parent=this,this.children.push(t),t.dispatchEvent(O_)):console.error("THREE.Object3D.add: object not an instance of THREE.Object3D.",t),this)}remove(t){if(arguments.length>1){for(let n=0;n<arguments.length;n++)this.remove(arguments[n]);return this}const e=this.children.indexOf(t);return e!==-1&&(t.parent=null,this.children.splice(e,1),t.dispatchEvent(F_)),this}removeFromParent(){const t=this.parent;return t!==null&&t.remove(this),this}clear(){return this.remove(...this.children)}attach(t){return this.updateWorldMatrix(!0,!1),ii.copy(this.matrixWorld).invert(),t.parent!==null&&(t.parent.updateWorldMatrix(!0,!1),ii.multiply(t.parent.matrixWorld)),t.applyMatrix4(ii),this.add(t),t.updateWorldMatrix(!1,!0),this}getObjectById(t){return this.getObjectByProperty("id",t)}getObjectByName(t){return this.getObjectByProperty("name",t)}getObjectByProperty(t,e){if(this[t]===e)return this;for(let n=0,r=this.children.length;n<r;n++){const o=this.children[n].getObjectByProperty(t,e);if(o!==void 0)return o}}getObjectsByProperty(t,e,n=[]){this[t]===e&&n.push(this);const r=this.children;for(let s=0,o=r.length;s<o;s++)r[s].getObjectsByProperty(t,e,n);return n}getWorldPosition(t){return this.updateWorldMatrix(!0,!1),t.setFromMatrixPosition(this.matrixWorld)}getWorldQuaternion(t){return this.updateWorldMatrix(!0,!1),this.matrixWorld.decompose(Vs,t,N_),t}getWorldScale(t){return this.updateWorldMatrix(!0,!1),this.matrixWorld.decompose(Vs,U_,t),t}getWorldDirection(t){this.updateWorldMatrix(!0,!1);const e=this.matrixWorld.elements;return t.set(e[8],e[9],e[10]).normalize()}raycast(){}traverse(t){t(this);const e=this.children;for(let n=0,r=e.length;n<r;n++)e[n].traverse(t)}traverseVisible(t){if(this.visible===!1)return;t(this);const e=this.children;for(let n=0,r=e.length;n<r;n++)e[n].traverseVisible(t)}traverseAncestors(t){const e=this.parent;e!==null&&(t(e),e.traverseAncestors(t))}updateMatrix(){this.matrix.compose(this.position,this.quaternion,this.scale),this.matrixWorldNeedsUpdate=!0}updateMatrixWorld(t){this.matrixAutoUpdate&&this.updateMatrix(),(this.matrixWorldNeedsUpdate||t)&&(this.parent===null?this.matrixWorld.copy(this.matrix):this.matrixWorld.multiplyMatrices(this.parent.matrixWorld,this.matrix),this.matrixWorldNeedsUpdate=!1,t=!0);const e=this.children;for(let n=0,r=e.length;n<r;n++){const s=e[n];(s.matrixWorldAutoUpdate===!0||t===!0)&&s.updateMatrixWorld(t)}}updateWorldMatrix(t,e){const n=this.parent;if(t===!0&&n!==null&&n.matrixWorldAutoUpdate===!0&&n.updateWorldMatrix(!0,!1),this.matrixAutoUpdate&&this.updateMatrix(),this.parent===null?this.matrixWorld.copy(this.matrix):this.matrixWorld.multiplyMatrices(this.parent.matrixWorld,this.matrix),e===!0){const r=this.children;for(let s=0,o=r.length;s<o;s++){const a=r[s];a.matrixWorldAutoUpdate===!0&&a.updateWorldMatrix(!1,!0)}}}toJSON(t){const e=t===void 0||typeof t=="string",n={};e&&(t={geometries:{},materials:{},textures:{},images:{},shapes:{},skeletons:{},animations:{},nodes:{}},n.metadata={version:4.6,type:"Object",generator:"Object3D.toJSON"});const r={};r.uuid=this.uuid,r.type=this.type,this.name!==""&&(r.name=this.name),this.castShadow===!0&&(r.castShadow=!0),this.receiveShadow===!0&&(r.receiveShadow=!0),this.visible===!1&&(r.visible=!1),this.frustumCulled===!1&&(r.frustumCulled=!1),this.renderOrder!==0&&(r.renderOrder=this.renderOrder),Object.keys(this.userData).length>0&&(r.userData=this.userData),r.layers=this.layers.mask,r.matrix=this.matrix.toArray(),r.up=this.up.toArray(),this.matrixAutoUpdate===!1&&(r.matrixAutoUpdate=!1),this.isInstancedMesh&&(r.type="InstancedMesh",r.count=this.count,r.instanceMatrix=this.instanceMatrix.toJSON(),this.instanceColor!==null&&(r.instanceColor=this.instanceColor.toJSON())),this.isBatchedMesh&&(r.type="BatchedMesh",r.perObjectFrustumCulled=this.perObjectFrustumCulled,r.sortObjects=this.sortObjects,r.drawRanges=this._drawRanges,r.reservedRanges=this._reservedRanges,r.visibility=this._visibility,r.active=this._active,r.bounds=this._bounds.map(a=>({boxInitialized:a.boxInitialized,boxMin:a.box.min.toArray(),boxMax:a.box.max.toArray(),sphereInitialized:a.sphereInitialized,sphereRadius:a.sphere.radius,sphereCenter:a.sphere.center.toArray()})),r.maxGeometryCount=this._maxGeometryCount,r.maxVertexCount=this._maxVertexCount,r.maxIndexCount=this._maxIndexCount,r.geometryInitialized=this._geometryInitialized,r.geometryCount=this._geometryCount,r.matricesTexture=this._matricesTexture.toJSON(t),this.boundingSphere!==null&&(r.boundingSphere={center:r.boundingSphere.center.toArray(),radius:r.boundingSphere.radius}),this.boundingBox!==null&&(r.boundingBox={min:r.boundingBox.min.toArray(),max:r.boundingBox.max.toArray()}));function s(a,c){return a[c.uuid]===void 0&&(a[c.uuid]=c.toJSON(t)),c.uuid}if(this.isScene)this.background&&(this.background.isColor?r.background=this.background.toJSON():this.background.isTexture&&(r.background=this.background.toJSON(t).uuid)),this.environment&&this.environment.isTexture&&this.environment.isRenderTargetTexture!==!0&&(r.environment=this.environment.toJSON(t).uuid);else if(this.isMesh||this.isLine||this.isPoints){r.geometry=s(t.geometries,this.geometry);const a=this.geometry.parameters;if(a!==void 0&&a.shapes!==void 0){const c=a.shapes;if(Array.isArray(c))for(let h=0,u=c.length;h<u;h++){const d=c[h];s(t.shapes,d)}else s(t.shapes,c)}}if(this.isSkinnedMesh&&(r.bindMode=this.bindMode,r.bindMatrix=this.bindMatrix.toArray(),this.skeleton!==void 0&&(s(t.skeletons,this.skeleton),r.skeleton=this.skeleton.uuid)),this.material!==void 0)if(Array.isArray(this.material)){const a=[];for(let c=0,h=this.material.length;c<h;c++)a.push(s(t.materials,this.material[c]));r.material=a}else r.material=s(t.materials,this.material);if(this.children.length>0){r.children=[];for(let a=0;a<this.children.length;a++)r.children.push(this.children[a].toJSON(t).object)}if(this.animations.length>0){r.animations=[];for(let a=0;a<this.animations.length;a++){const c=this.animations[a];r.animations.push(s(t.animations,c))}}if(e){const a=o(t.geometries),c=o(t.materials),h=o(t.textures),u=o(t.images),d=o(t.shapes),f=o(t.skeletons),p=o(t.animations),y=o(t.nodes);a.length>0&&(n.geometries=a),c.length>0&&(n.materials=c),h.length>0&&(n.textures=h),u.length>0&&(n.images=u),d.length>0&&(n.shapes=d),f.length>0&&(n.skeletons=f),p.length>0&&(n.animations=p),y.length>0&&(n.nodes=y)}return n.object=r,n;function o(a){const c=[];for(const h in a){const u=a[h];delete u.metadata,c.push(u)}return c}}clone(t){return new this.constructor().copy(this,t)}copy(t,e=!0){if(this.name=t.name,this.up.copy(t.up),this.position.copy(t.position),this.rotation.order=t.rotation.order,this.quaternion.copy(t.quaternion),this.scale.copy(t.scale),this.matrix.copy(t.matrix),this.matrixWorld.copy(t.matrixWorld),this.matrixAutoUpdate=t.matrixAutoUpdate,this.matrixWorldAutoUpdate=t.matrixWorldAutoUpdate,this.matrixWorldNeedsUpdate=t.matrixWorldNeedsUpdate,this.layers.mask=t.layers.mask,this.visible=t.visible,this.castShadow=t.castShadow,this.receiveShadow=t.receiveShadow,this.frustumCulled=t.frustumCulled,this.renderOrder=t.renderOrder,this.animations=t.animations.slice(),this.userData=JSON.parse(JSON.stringify(t.userData)),e===!0)for(let n=0;n<t.children.length;n++){const r=t.children[n];this.add(r.clone())}return this}}je.DEFAULT_UP=new k(0,1,0);je.DEFAULT_MATRIX_AUTO_UPDATE=!0;je.DEFAULT_MATRIX_WORLD_AUTO_UPDATE=!0;const Hn=new k,ri=new k,Rc=new k,si=new k,Nr=new k,Ur=new k,$u=new k,Cc=new k,Ic=new k,Pc=new k;let Ho=!1;class Dn{constructor(t=new k,e=new k,n=new k){this.a=t,this.b=e,this.c=n}static getNormal(t,e,n,r){r.subVectors(n,e),Hn.subVectors(t,e),r.cross(Hn);const s=r.lengthSq();return s>0?r.multiplyScalar(1/Math.sqrt(s)):r.set(0,0,0)}static getBarycoord(t,e,n,r,s){Hn.subVectors(r,e),ri.subVectors(n,e),Rc.subVectors(t,e);const o=Hn.dot(Hn),a=Hn.dot(ri),c=Hn.dot(Rc),h=ri.dot(ri),u=ri.dot(Rc),d=o*h-a*a;if(d===0)return s.set(0,0,0),null;const f=1/d,p=(h*c-a*u)*f,y=(o*u-a*c)*f;return s.set(1-p-y,y,p)}static containsPoint(t,e,n,r){return this.getBarycoord(t,e,n,r,si)===null?!1:si.x>=0&&si.y>=0&&si.x+si.y<=1}static getUV(t,e,n,r,s,o,a,c){return Ho===!1&&(console.warn("THREE.Triangle.getUV() has been renamed to THREE.Triangle.getInterpolation()."),Ho=!0),this.getInterpolation(t,e,n,r,s,o,a,c)}static getInterpolation(t,e,n,r,s,o,a,c){return this.getBarycoord(t,e,n,r,si)===null?(c.x=0,c.y=0,"z"in c&&(c.z=0),"w"in c&&(c.w=0),null):(c.setScalar(0),c.addScaledVector(s,si.x),c.addScaledVector(o,si.y),c.addScaledVector(a,si.z),c)}static isFrontFacing(t,e,n,r){return Hn.subVectors(n,e),ri.subVectors(t,e),Hn.cross(ri).dot(r)<0}set(t,e,n){return this.a.copy(t),this.b.copy(e),this.c.copy(n),this}setFromPointsAndIndices(t,e,n,r){return this.a.copy(t[e]),this.b.copy(t[n]),this.c.copy(t[r]),this}setFromAttributeAndIndices(t,e,n,r){return this.a.fromBufferAttribute(t,e),this.b.fromBufferAttribute(t,n),this.c.fromBufferAttribute(t,r),this}clone(){return new this.constructor().copy(this)}copy(t){return this.a.copy(t.a),this.b.copy(t.b),this.c.copy(t.c),this}getArea(){return Hn.subVectors(this.c,this.b),ri.subVectors(this.a,this.b),Hn.cross(ri).length()*.5}getMidpoint(t){return t.addVectors(this.a,this.b).add(this.c).multiplyScalar(1/3)}getNormal(t){return Dn.getNormal(this.a,this.b,this.c,t)}getPlane(t){return t.setFromCoplanarPoints(this.a,this.b,this.c)}getBarycoord(t,e){return Dn.getBarycoord(t,this.a,this.b,this.c,e)}getUV(t,e,n,r,s){return Ho===!1&&(console.warn("THREE.Triangle.getUV() has been renamed to THREE.Triangle.getInterpolation()."),Ho=!0),Dn.getInterpolation(t,this.a,this.b,this.c,e,n,r,s)}getInterpolation(t,e,n,r,s){return Dn.getInterpolation(t,this.a,this.b,this.c,e,n,r,s)}containsPoint(t){return Dn.containsPoint(t,this.a,this.b,this.c)}isFrontFacing(t){return Dn.isFrontFacing(this.a,this.b,this.c,t)}intersectsBox(t){return t.intersectsTriangle(this)}closestPointToPoint(t,e){const n=this.a,r=this.b,s=this.c;let o,a;Nr.subVectors(r,n),Ur.subVectors(s,n),Cc.subVectors(t,n);const c=Nr.dot(Cc),h=Ur.dot(Cc);if(c<=0&&h<=0)return e.copy(n);Ic.subVectors(t,r);const u=Nr.dot(Ic),d=Ur.dot(Ic);if(u>=0&&d<=u)return e.copy(r);const f=c*d-u*h;if(f<=0&&c>=0&&u<=0)return o=c/(c-u),e.copy(n).addScaledVector(Nr,o);Pc.subVectors(t,s);const p=Nr.dot(Pc),y=Ur.dot(Pc);if(y>=0&&p<=y)return e.copy(s);const E=p*h-c*y;if(E<=0&&h>=0&&y<=0)return a=h/(h-y),e.copy(n).addScaledVector(Ur,a);const _=u*y-p*d;if(_<=0&&d-u>=0&&p-y>=0)return $u.subVectors(s,r),a=(d-u)/(d-u+(p-y)),e.copy(r).addScaledVector($u,a);const m=1/(_+E+f);return o=E*m,a=f*m,e.copy(n).addScaledVector(Nr,o).addScaledVector(Ur,a)}equals(t){return t.a.equals(this.a)&&t.b.equals(this.b)&&t.c.equals(this.c)}}const cp={aliceblue:15792383,antiquewhite:16444375,aqua:65535,aquamarine:8388564,azure:15794175,beige:16119260,bisque:16770244,black:0,blanchedalmond:16772045,blue:255,blueviolet:9055202,brown:10824234,burlywood:14596231,cadetblue:6266528,chartreuse:8388352,chocolate:13789470,coral:16744272,cornflowerblue:6591981,cornsilk:16775388,crimson:14423100,cyan:65535,darkblue:139,darkcyan:35723,darkgoldenrod:12092939,darkgray:11119017,darkgreen:25600,darkgrey:11119017,darkkhaki:12433259,darkmagenta:9109643,darkolivegreen:5597999,darkorange:16747520,darkorchid:10040012,darkred:9109504,darksalmon:15308410,darkseagreen:9419919,darkslateblue:4734347,darkslategray:3100495,darkslategrey:3100495,darkturquoise:52945,darkviolet:9699539,deeppink:16716947,deepskyblue:49151,dimgray:6908265,dimgrey:6908265,dodgerblue:2003199,firebrick:11674146,floralwhite:16775920,forestgreen:2263842,fuchsia:16711935,gainsboro:14474460,ghostwhite:16316671,gold:16766720,goldenrod:14329120,gray:8421504,green:32768,greenyellow:11403055,grey:8421504,honeydew:15794160,hotpink:16738740,indianred:13458524,indigo:4915330,ivory:16777200,khaki:15787660,lavender:15132410,lavenderblush:16773365,lawngreen:8190976,lemonchiffon:16775885,lightblue:11393254,lightcoral:15761536,lightcyan:14745599,lightgoldenrodyellow:16448210,lightgray:13882323,lightgreen:9498256,lightgrey:13882323,lightpink:16758465,lightsalmon:16752762,lightseagreen:2142890,lightskyblue:8900346,lightslategray:7833753,lightslategrey:7833753,lightsteelblue:11584734,lightyellow:16777184,lime:65280,limegreen:3329330,linen:16445670,magenta:16711935,maroon:8388608,mediumaquamarine:6737322,mediumblue:205,mediumorchid:12211667,mediumpurple:9662683,mediumseagreen:3978097,mediumslateblue:8087790,mediumspringgreen:64154,mediumturquoise:4772300,mediumvioletred:13047173,midnightblue:1644912,mintcream:16121850,mistyrose:16770273,moccasin:16770229,navajowhite:16768685,navy:128,oldlace:16643558,olive:8421376,olivedrab:7048739,orange:16753920,orangered:16729344,orchid:14315734,palegoldenrod:15657130,palegreen:10025880,paleturquoise:11529966,palevioletred:14381203,papayawhip:16773077,peachpuff:16767673,peru:13468991,pink:16761035,plum:14524637,powderblue:11591910,purple:8388736,rebeccapurple:6697881,red:16711680,rosybrown:12357519,royalblue:4286945,saddlebrown:9127187,salmon:16416882,sandybrown:16032864,seagreen:3050327,seashell:16774638,sienna:10506797,silver:12632256,skyblue:8900331,slateblue:6970061,slategray:7372944,slategrey:7372944,snow:16775930,springgreen:65407,steelblue:4620980,tan:13808780,teal:32896,thistle:14204888,tomato:16737095,turquoise:4251856,violet:15631086,wheat:16113331,white:16777215,whitesmoke:16119285,yellow:16776960,yellowgreen:10145074},Mi={h:0,s:0,l:0},zo={h:0,s:0,l:0};function Dc(i,t,e){return e<0&&(e+=1),e>1&&(e-=1),e<1/6?i+(t-i)*6*e:e<1/2?t:e<2/3?i+(t-i)*6*(2/3-e):i}class oe{constructor(t,e,n){return this.isColor=!0,this.r=1,this.g=1,this.b=1,this.set(t,e,n)}set(t,e,n){if(e===void 0&&n===void 0){const r=t;r&&r.isColor?this.copy(r):typeof r=="number"?this.setHex(r):typeof r=="string"&&this.setStyle(r)}else this.setRGB(t,e,n);return this}setScalar(t){return this.r=t,this.g=t,this.b=t,this}setHex(t,e=Ze){return t=Math.floor(t),this.r=(t>>16&255)/255,this.g=(t>>8&255)/255,this.b=(t&255)/255,ve.toWorkingColorSpace(this,e),this}setRGB(t,e,n,r=ve.workingColorSpace){return this.r=t,this.g=e,this.b=n,ve.toWorkingColorSpace(this,r),this}setHSL(t,e,n,r=ve.workingColorSpace){if(t=S_(t,1),e=mn(e,0,1),n=mn(n,0,1),e===0)this.r=this.g=this.b=n;else{const s=n<=.5?n*(1+e):n+e-n*e,o=2*n-s;this.r=Dc(o,s,t+1/3),this.g=Dc(o,s,t),this.b=Dc(o,s,t-1/3)}return ve.toWorkingColorSpace(this,r),this}setStyle(t,e=Ze){function n(s){s!==void 0&&parseFloat(s)<1&&console.warn("THREE.Color: Alpha component of "+t+" will be ignored.")}let r;if(r=/^(\w+)\(([^\)]*)\)/.exec(t)){let s;const o=r[1],a=r[2];switch(o){case"rgb":case"rgba":if(s=/^\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*(?:,\s*(\d*\.?\d+)\s*)?$/.exec(a))return n(s[4]),this.setRGB(Math.min(255,parseInt(s[1],10))/255,Math.min(255,parseInt(s[2],10))/255,Math.min(255,parseInt(s[3],10))/255,e);if(s=/^\s*(\d+)\%\s*,\s*(\d+)\%\s*,\s*(\d+)\%\s*(?:,\s*(\d*\.?\d+)\s*)?$/.exec(a))return n(s[4]),this.setRGB(Math.min(100,parseInt(s[1],10))/100,Math.min(100,parseInt(s[2],10))/100,Math.min(100,parseInt(s[3],10))/100,e);break;case"hsl":case"hsla":if(s=/^\s*(\d*\.?\d+)\s*,\s*(\d*\.?\d+)\%\s*,\s*(\d*\.?\d+)\%\s*(?:,\s*(\d*\.?\d+)\s*)?$/.exec(a))return n(s[4]),this.setHSL(parseFloat(s[1])/360,parseFloat(s[2])/100,parseFloat(s[3])/100,e);break;default:console.warn("THREE.Color: Unknown color model "+t)}}else if(r=/^\#([A-Fa-f\d]+)$/.exec(t)){const s=r[1],o=s.length;if(o===3)return this.setRGB(parseInt(s.charAt(0),16)/15,parseInt(s.charAt(1),16)/15,parseInt(s.charAt(2),16)/15,e);if(o===6)return this.setHex(parseInt(s,16),e);console.warn("THREE.Color: Invalid hex color "+t)}else if(t&&t.length>0)return this.setColorName(t,e);return this}setColorName(t,e=Ze){const n=cp[t.toLowerCase()];return n!==void 0?this.setHex(n,e):console.warn("THREE.Color: Unknown color "+t),this}clone(){return new this.constructor(this.r,this.g,this.b)}copy(t){return this.r=t.r,this.g=t.g,this.b=t.b,this}copySRGBToLinear(t){return this.r=es(t.r),this.g=es(t.g),this.b=es(t.b),this}copyLinearToSRGB(t){return this.r=Ec(t.r),this.g=Ec(t.g),this.b=Ec(t.b),this}convertSRGBToLinear(){return this.copySRGBToLinear(this),this}convertLinearToSRGB(){return this.copyLinearToSRGB(this),this}getHex(t=Ze){return ve.fromWorkingColorSpace(an.copy(this),t),Math.round(mn(an.r*255,0,255))*65536+Math.round(mn(an.g*255,0,255))*256+Math.round(mn(an.b*255,0,255))}getHexString(t=Ze){return("000000"+this.getHex(t).toString(16)).slice(-6)}getHSL(t,e=ve.workingColorSpace){ve.fromWorkingColorSpace(an.copy(this),e);const n=an.r,r=an.g,s=an.b,o=Math.max(n,r,s),a=Math.min(n,r,s);let c,h;const u=(a+o)/2;if(a===o)c=0,h=0;else{const d=o-a;switch(h=u<=.5?d/(o+a):d/(2-o-a),o){case n:c=(r-s)/d+(r<s?6:0);break;case r:c=(s-n)/d+2;break;case s:c=(n-r)/d+4;break}c/=6}return t.h=c,t.s=h,t.l=u,t}getRGB(t,e=ve.workingColorSpace){return ve.fromWorkingColorSpace(an.copy(this),e),t.r=an.r,t.g=an.g,t.b=an.b,t}getStyle(t=Ze){ve.fromWorkingColorSpace(an.copy(this),t);const e=an.r,n=an.g,r=an.b;return t!==Ze?`color(${t} ${e.toFixed(3)} ${n.toFixed(3)} ${r.toFixed(3)})`:`rgb(${Math.round(e*255)},${Math.round(n*255)},${Math.round(r*255)})`}offsetHSL(t,e,n){return this.getHSL(Mi),this.setHSL(Mi.h+t,Mi.s+e,Mi.l+n)}add(t){return this.r+=t.r,this.g+=t.g,this.b+=t.b,this}addColors(t,e){return this.r=t.r+e.r,this.g=t.g+e.g,this.b=t.b+e.b,this}addScalar(t){return this.r+=t,this.g+=t,this.b+=t,this}sub(t){return this.r=Math.max(0,this.r-t.r),this.g=Math.max(0,this.g-t.g),this.b=Math.max(0,this.b-t.b),this}multiply(t){return this.r*=t.r,this.g*=t.g,this.b*=t.b,this}multiplyScalar(t){return this.r*=t,this.g*=t,this.b*=t,this}lerp(t,e){return this.r+=(t.r-this.r)*e,this.g+=(t.g-this.g)*e,this.b+=(t.b-this.b)*e,this}lerpColors(t,e,n){return this.r=t.r+(e.r-t.r)*n,this.g=t.g+(e.g-t.g)*n,this.b=t.b+(e.b-t.b)*n,this}lerpHSL(t,e){this.getHSL(Mi),t.getHSL(zo);const n=vc(Mi.h,zo.h,e),r=vc(Mi.s,zo.s,e),s=vc(Mi.l,zo.l,e);return this.setHSL(n,r,s),this}setFromVector3(t){return this.r=t.x,this.g=t.y,this.b=t.z,this}applyMatrix3(t){const e=this.r,n=this.g,r=this.b,s=t.elements;return this.r=s[0]*e+s[3]*n+s[6]*r,this.g=s[1]*e+s[4]*n+s[7]*r,this.b=s[2]*e+s[5]*n+s[8]*r,this}equals(t){return t.r===this.r&&t.g===this.g&&t.b===this.b}fromArray(t,e=0){return this.r=t[e],this.g=t[e+1],this.b=t[e+2],this}toArray(t=[],e=0){return t[e]=this.r,t[e+1]=this.g,t[e+2]=this.b,t}fromBufferAttribute(t,e){return this.r=t.getX(e),this.g=t.getY(e),this.b=t.getZ(e),this}toJSON(){return this.getHex()}*[Symbol.iterator](){yield this.r,yield this.g,yield this.b}}const an=new oe;oe.NAMES=cp;let V_=0;class gs extends gr{constructor(){super(),this.isMaterial=!0,Object.defineProperty(this,"id",{value:V_++}),this.uuid=Ui(),this.name="",this.type="Material",this.blending=ts,this.side=ui,this.vertexColors=!1,this.opacity=1,this.transparent=!1,this.alphaHash=!1,this.blendSrc=rl,this.blendDst=sl,this.blendEquation=nr,this.blendSrcAlpha=null,this.blendDstAlpha=null,this.blendEquationAlpha=null,this.blendColor=new oe(0,0,0),this.blendAlpha=0,this.depthFunc=xa,this.depthTest=!0,this.depthWrite=!0,this.stencilWriteMask=255,this.stencilFunc=Ou,this.stencilRef=0,this.stencilFuncMask=255,this.stencilFail=Ar,this.stencilZFail=Ar,this.stencilZPass=Ar,this.stencilWrite=!1,this.clippingPlanes=null,this.clipIntersection=!1,this.clipShadows=!1,this.shadowSide=null,this.colorWrite=!0,this.precision=null,this.polygonOffset=!1,this.polygonOffsetFactor=0,this.polygonOffsetUnits=0,this.dithering=!1,this.alphaToCoverage=!1,this.premultipliedAlpha=!1,this.forceSinglePass=!1,this.visible=!0,this.toneMapped=!0,this.userData={},this.version=0,this._alphaTest=0}get alphaTest(){return this._alphaTest}set alphaTest(t){this._alphaTest>0!=t>0&&this.version++,this._alphaTest=t}onBuild(){}onBeforeRender(){}onBeforeCompile(){}customProgramCacheKey(){return this.onBeforeCompile.toString()}setValues(t){if(t!==void 0)for(const e in t){const n=t[e];if(n===void 0){console.warn(`THREE.Material: parameter '${e}' has value of undefined.`);continue}const r=this[e];if(r===void 0){console.warn(`THREE.Material: '${e}' is not a property of THREE.${this.type}.`);continue}r&&r.isColor?r.set(n):r&&r.isVector3&&n&&n.isVector3?r.copy(n):this[e]=n}}toJSON(t){const e=t===void 0||typeof t=="string";e&&(t={textures:{},images:{}});const n={metadata:{version:4.6,type:"Material",generator:"Material.toJSON"}};n.uuid=this.uuid,n.type=this.type,this.name!==""&&(n.name=this.name),this.color&&this.color.isColor&&(n.color=this.color.getHex()),this.roughness!==void 0&&(n.roughness=this.roughness),this.metalness!==void 0&&(n.metalness=this.metalness),this.sheen!==void 0&&(n.sheen=this.sheen),this.sheenColor&&this.sheenColor.isColor&&(n.sheenColor=this.sheenColor.getHex()),this.sheenRoughness!==void 0&&(n.sheenRoughness=this.sheenRoughness),this.emissive&&this.emissive.isColor&&(n.emissive=this.emissive.getHex()),this.emissiveIntensity&&this.emissiveIntensity!==1&&(n.emissiveIntensity=this.emissiveIntensity),this.specular&&this.specular.isColor&&(n.specular=this.specular.getHex()),this.specularIntensity!==void 0&&(n.specularIntensity=this.specularIntensity),this.specularColor&&this.specularColor.isColor&&(n.specularColor=this.specularColor.getHex()),this.shininess!==void 0&&(n.shininess=this.shininess),this.clearcoat!==void 0&&(n.clearcoat=this.clearcoat),this.clearcoatRoughness!==void 0&&(n.clearcoatRoughness=this.clearcoatRoughness),this.clearcoatMap&&this.clearcoatMap.isTexture&&(n.clearcoatMap=this.clearcoatMap.toJSON(t).uuid),this.clearcoatRoughnessMap&&this.clearcoatRoughnessMap.isTexture&&(n.clearcoatRoughnessMap=this.clearcoatRoughnessMap.toJSON(t).uuid),this.clearcoatNormalMap&&this.clearcoatNormalMap.isTexture&&(n.clearcoatNormalMap=this.clearcoatNormalMap.toJSON(t).uuid,n.clearcoatNormalScale=this.clearcoatNormalScale.toArray()),this.iridescence!==void 0&&(n.iridescence=this.iridescence),this.iridescenceIOR!==void 0&&(n.iridescenceIOR=this.iridescenceIOR),this.iridescenceThicknessRange!==void 0&&(n.iridescenceThicknessRange=this.iridescenceThicknessRange),this.iridescenceMap&&this.iridescenceMap.isTexture&&(n.iridescenceMap=this.iridescenceMap.toJSON(t).uuid),this.iridescenceThicknessMap&&this.iridescenceThicknessMap.isTexture&&(n.iridescenceThicknessMap=this.iridescenceThicknessMap.toJSON(t).uuid),this.anisotropy!==void 0&&(n.anisotropy=this.anisotropy),this.anisotropyRotation!==void 0&&(n.anisotropyRotation=this.anisotropyRotation),this.anisotropyMap&&this.anisotropyMap.isTexture&&(n.anisotropyMap=this.anisotropyMap.toJSON(t).uuid),this.map&&this.map.isTexture&&(n.map=this.map.toJSON(t).uuid),this.matcap&&this.matcap.isTexture&&(n.matcap=this.matcap.toJSON(t).uuid),this.alphaMap&&this.alphaMap.isTexture&&(n.alphaMap=this.alphaMap.toJSON(t).uuid),this.lightMap&&this.lightMap.isTexture&&(n.lightMap=this.lightMap.toJSON(t).uuid,n.lightMapIntensity=this.lightMapIntensity),this.aoMap&&this.aoMap.isTexture&&(n.aoMap=this.aoMap.toJSON(t).uuid,n.aoMapIntensity=this.aoMapIntensity),this.bumpMap&&this.bumpMap.isTexture&&(n.bumpMap=this.bumpMap.toJSON(t).uuid,n.bumpScale=this.bumpScale),this.normalMap&&this.normalMap.isTexture&&(n.normalMap=this.normalMap.toJSON(t).uuid,n.normalMapType=this.normalMapType,n.normalScale=this.normalScale.toArray()),this.displacementMap&&this.displacementMap.isTexture&&(n.displacementMap=this.displacementMap.toJSON(t).uuid,n.displacementScale=this.displacementScale,n.displacementBias=this.displacementBias),this.roughnessMap&&this.roughnessMap.isTexture&&(n.roughnessMap=this.roughnessMap.toJSON(t).uuid),this.metalnessMap&&this.metalnessMap.isTexture&&(n.metalnessMap=this.metalnessMap.toJSON(t).uuid),this.emissiveMap&&this.emissiveMap.isTexture&&(n.emissiveMap=this.emissiveMap.toJSON(t).uuid),this.specularMap&&this.specularMap.isTexture&&(n.specularMap=this.specularMap.toJSON(t).uuid),this.specularIntensityMap&&this.specularIntensityMap.isTexture&&(n.specularIntensityMap=this.specularIntensityMap.toJSON(t).uuid),this.specularColorMap&&this.specularColorMap.isTexture&&(n.specularColorMap=this.specularColorMap.toJSON(t).uuid),this.envMap&&this.envMap.isTexture&&(n.envMap=this.envMap.toJSON(t).uuid,this.combine!==void 0&&(n.combine=this.combine)),this.envMapIntensity!==void 0&&(n.envMapIntensity=this.envMapIntensity),this.reflectivity!==void 0&&(n.reflectivity=this.reflectivity),this.refractionRatio!==void 0&&(n.refractionRatio=this.refractionRatio),this.gradientMap&&this.gradientMap.isTexture&&(n.gradientMap=this.gradientMap.toJSON(t).uuid),this.transmission!==void 0&&(n.transmission=this.transmission),this.transmissionMap&&this.transmissionMap.isTexture&&(n.transmissionMap=this.transmissionMap.toJSON(t).uuid),this.thickness!==void 0&&(n.thickness=this.thickness),this.thicknessMap&&this.thicknessMap.isTexture&&(n.thicknessMap=this.thicknessMap.toJSON(t).uuid),this.attenuationDistance!==void 0&&this.attenuationDistance!==1/0&&(n.attenuationDistance=this.attenuationDistance),this.attenuationColor!==void 0&&(n.attenuationColor=this.attenuationColor.getHex()),this.size!==void 0&&(n.size=this.size),this.shadowSide!==null&&(n.shadowSide=this.shadowSide),this.sizeAttenuation!==void 0&&(n.sizeAttenuation=this.sizeAttenuation),this.blending!==ts&&(n.blending=this.blending),this.side!==ui&&(n.side=this.side),this.vertexColors===!0&&(n.vertexColors=!0),this.opacity<1&&(n.opacity=this.opacity),this.transparent===!0&&(n.transparent=!0),this.blendSrc!==rl&&(n.blendSrc=this.blendSrc),this.blendDst!==sl&&(n.blendDst=this.blendDst),this.blendEquation!==nr&&(n.blendEquation=this.blendEquation),this.blendSrcAlpha!==null&&(n.blendSrcAlpha=this.blendSrcAlpha),this.blendDstAlpha!==null&&(n.blendDstAlpha=this.blendDstAlpha),this.blendEquationAlpha!==null&&(n.blendEquationAlpha=this.blendEquationAlpha),this.blendColor&&this.blendColor.isColor&&(n.blendColor=this.blendColor.getHex()),this.blendAlpha!==0&&(n.blendAlpha=this.blendAlpha),this.depthFunc!==xa&&(n.depthFunc=this.depthFunc),this.depthTest===!1&&(n.depthTest=this.depthTest),this.depthWrite===!1&&(n.depthWrite=this.depthWrite),this.colorWrite===!1&&(n.colorWrite=this.colorWrite),this.stencilWriteMask!==255&&(n.stencilWriteMask=this.stencilWriteMask),this.stencilFunc!==Ou&&(n.stencilFunc=this.stencilFunc),this.stencilRef!==0&&(n.stencilRef=this.stencilRef),this.stencilFuncMask!==255&&(n.stencilFuncMask=this.stencilFuncMask),this.stencilFail!==Ar&&(n.stencilFail=this.stencilFail),this.stencilZFail!==Ar&&(n.stencilZFail=this.stencilZFail),this.stencilZPass!==Ar&&(n.stencilZPass=this.stencilZPass),this.stencilWrite===!0&&(n.stencilWrite=this.stencilWrite),this.rotation!==void 0&&this.rotation!==0&&(n.rotation=this.rotation),this.polygonOffset===!0&&(n.polygonOffset=!0),this.polygonOffsetFactor!==0&&(n.polygonOffsetFactor=this.polygonOffsetFactor),this.polygonOffsetUnits!==0&&(n.polygonOffsetUnits=this.polygonOffsetUnits),this.linewidth!==void 0&&this.linewidth!==1&&(n.linewidth=this.linewidth),this.dashSize!==void 0&&(n.dashSize=this.dashSize),this.gapSize!==void 0&&(n.gapSize=this.gapSize),this.scale!==void 0&&(n.scale=this.scale),this.dithering===!0&&(n.dithering=!0),this.alphaTest>0&&(n.alphaTest=this.alphaTest),this.alphaHash===!0&&(n.alphaHash=!0),this.alphaToCoverage===!0&&(n.alphaToCoverage=!0),this.premultipliedAlpha===!0&&(n.premultipliedAlpha=!0),this.forceSinglePass===!0&&(n.forceSinglePass=!0),this.wireframe===!0&&(n.wireframe=!0),this.wireframeLinewidth>1&&(n.wireframeLinewidth=this.wireframeLinewidth),this.wireframeLinecap!=="round"&&(n.wireframeLinecap=this.wireframeLinecap),this.wireframeLinejoin!=="round"&&(n.wireframeLinejoin=this.wireframeLinejoin),this.flatShading===!0&&(n.flatShading=!0),this.visible===!1&&(n.visible=!1),this.toneMapped===!1&&(n.toneMapped=!1),this.fog===!1&&(n.fog=!1),Object.keys(this.userData).length>0&&(n.userData=this.userData);function r(s){const o=[];for(const a in s){const c=s[a];delete c.metadata,o.push(c)}return o}if(e){const s=r(t.textures),o=r(t.images);s.length>0&&(n.textures=s),o.length>0&&(n.images=o)}return n}clone(){return new this.constructor().copy(this)}copy(t){this.name=t.name,this.blending=t.blending,this.side=t.side,this.vertexColors=t.vertexColors,this.opacity=t.opacity,this.transparent=t.transparent,this.blendSrc=t.blendSrc,this.blendDst=t.blendDst,this.blendEquation=t.blendEquation,this.blendSrcAlpha=t.blendSrcAlpha,this.blendDstAlpha=t.blendDstAlpha,this.blendEquationAlpha=t.blendEquationAlpha,this.blendColor.copy(t.blendColor),this.blendAlpha=t.blendAlpha,this.depthFunc=t.depthFunc,this.depthTest=t.depthTest,this.depthWrite=t.depthWrite,this.stencilWriteMask=t.stencilWriteMask,this.stencilFunc=t.stencilFunc,this.stencilRef=t.stencilRef,this.stencilFuncMask=t.stencilFuncMask,this.stencilFail=t.stencilFail,this.stencilZFail=t.stencilZFail,this.stencilZPass=t.stencilZPass,this.stencilWrite=t.stencilWrite;const e=t.clippingPlanes;let n=null;if(e!==null){const r=e.length;n=new Array(r);for(let s=0;s!==r;++s)n[s]=e[s].clone()}return this.clippingPlanes=n,this.clipIntersection=t.clipIntersection,this.clipShadows=t.clipShadows,this.shadowSide=t.shadowSide,this.colorWrite=t.colorWrite,this.precision=t.precision,this.polygonOffset=t.polygonOffset,this.polygonOffsetFactor=t.polygonOffsetFactor,this.polygonOffsetUnits=t.polygonOffsetUnits,this.dithering=t.dithering,this.alphaTest=t.alphaTest,this.alphaHash=t.alphaHash,this.alphaToCoverage=t.alphaToCoverage,this.premultipliedAlpha=t.premultipliedAlpha,this.forceSinglePass=t.forceSinglePass,this.visible=t.visible,this.toneMapped=t.toneMapped,this.userData=JSON.parse(JSON.stringify(t.userData)),this}dispose(){this.dispatchEvent({type:"dispose"})}set needsUpdate(t){t===!0&&this.version++}}class bn extends gs{constructor(t){super(),this.isMeshBasicMaterial=!0,this.type="MeshBasicMaterial",this.color=new oe(16777215),this.map=null,this.lightMap=null,this.lightMapIntensity=1,this.aoMap=null,this.aoMapIntensity=1,this.specularMap=null,this.alphaMap=null,this.envMap=null,this.combine=qf,this.reflectivity=1,this.refractionRatio=.98,this.wireframe=!1,this.wireframeLinewidth=1,this.wireframeLinecap="round",this.wireframeLinejoin="round",this.fog=!0,this.setValues(t)}copy(t){return super.copy(t),this.color.copy(t.color),this.map=t.map,this.lightMap=t.lightMap,this.lightMapIntensity=t.lightMapIntensity,this.aoMap=t.aoMap,this.aoMapIntensity=t.aoMapIntensity,this.specularMap=t.specularMap,this.alphaMap=t.alphaMap,this.envMap=t.envMap,this.combine=t.combine,this.reflectivity=t.reflectivity,this.refractionRatio=t.refractionRatio,this.wireframe=t.wireframe,this.wireframeLinewidth=t.wireframeLinewidth,this.wireframeLinecap=t.wireframeLinecap,this.wireframeLinejoin=t.wireframeLinejoin,this.fog=t.fog,this}}const Oe=new k,Go=new Vt;class xn{constructor(t,e,n=!1){if(Array.isArray(t))throw new TypeError("THREE.BufferAttribute: array should be a Typed Array.");this.isBufferAttribute=!0,this.name="",this.array=t,this.itemSize=e,this.count=t!==void 0?t.length/e:0,this.normalized=n,this.usage=hl,this._updateRange={offset:0,count:-1},this.updateRanges=[],this.gpuType=Ci,this.version=0}onUploadCallback(){}set needsUpdate(t){t===!0&&this.version++}get updateRange(){return console.warn("THREE.BufferAttribute: updateRange() is deprecated and will be removed in r169. Use addUpdateRange() instead."),this._updateRange}setUsage(t){return this.usage=t,this}addUpdateRange(t,e){this.updateRanges.push({start:t,count:e})}clearUpdateRanges(){this.updateRanges.length=0}copy(t){return this.name=t.name,this.array=new t.array.constructor(t.array),this.itemSize=t.itemSize,this.count=t.count,this.normalized=t.normalized,this.usage=t.usage,this.gpuType=t.gpuType,this}copyAt(t,e,n){t*=this.itemSize,n*=e.itemSize;for(let r=0,s=this.itemSize;r<s;r++)this.array[t+r]=e.array[n+r];return this}copyArray(t){return this.array.set(t),this}applyMatrix3(t){if(this.itemSize===2)for(let e=0,n=this.count;e<n;e++)Go.fromBufferAttribute(this,e),Go.applyMatrix3(t),this.setXY(e,Go.x,Go.y);else if(this.itemSize===3)for(let e=0,n=this.count;e<n;e++)Oe.fromBufferAttribute(this,e),Oe.applyMatrix3(t),this.setXYZ(e,Oe.x,Oe.y,Oe.z);return this}applyMatrix4(t){for(let e=0,n=this.count;e<n;e++)Oe.fromBufferAttribute(this,e),Oe.applyMatrix4(t),this.setXYZ(e,Oe.x,Oe.y,Oe.z);return this}applyNormalMatrix(t){for(let e=0,n=this.count;e<n;e++)Oe.fromBufferAttribute(this,e),Oe.applyNormalMatrix(t),this.setXYZ(e,Oe.x,Oe.y,Oe.z);return this}transformDirection(t){for(let e=0,n=this.count;e<n;e++)Oe.fromBufferAttribute(this,e),Oe.transformDirection(t),this.setXYZ(e,Oe.x,Oe.y,Oe.z);return this}set(t,e=0){return this.array.set(t,e),this}getComponent(t,e){let n=this.array[t*this.itemSize+e];return this.normalized&&(n=ai(n,this.array)),n}setComponent(t,e,n){return this.normalized&&(n=xe(n,this.array)),this.array[t*this.itemSize+e]=n,this}getX(t){let e=this.array[t*this.itemSize];return this.normalized&&(e=ai(e,this.array)),e}setX(t,e){return this.normalized&&(e=xe(e,this.array)),this.array[t*this.itemSize]=e,this}getY(t){let e=this.array[t*this.itemSize+1];return this.normalized&&(e=ai(e,this.array)),e}setY(t,e){return this.normalized&&(e=xe(e,this.array)),this.array[t*this.itemSize+1]=e,this}getZ(t){let e=this.array[t*this.itemSize+2];return this.normalized&&(e=ai(e,this.array)),e}setZ(t,e){return this.normalized&&(e=xe(e,this.array)),this.array[t*this.itemSize+2]=e,this}getW(t){let e=this.array[t*this.itemSize+3];return this.normalized&&(e=ai(e,this.array)),e}setW(t,e){return this.normalized&&(e=xe(e,this.array)),this.array[t*this.itemSize+3]=e,this}setXY(t,e,n){return t*=this.itemSize,this.normalized&&(e=xe(e,this.array),n=xe(n,this.array)),this.array[t+0]=e,this.array[t+1]=n,this}setXYZ(t,e,n,r){return t*=this.itemSize,this.normalized&&(e=xe(e,this.array),n=xe(n,this.array),r=xe(r,this.array)),this.array[t+0]=e,this.array[t+1]=n,this.array[t+2]=r,this}setXYZW(t,e,n,r,s){return t*=this.itemSize,this.normalized&&(e=xe(e,this.array),n=xe(n,this.array),r=xe(r,this.array),s=xe(s,this.array)),this.array[t+0]=e,this.array[t+1]=n,this.array[t+2]=r,this.array[t+3]=s,this}onUpload(t){return this.onUploadCallback=t,this}clone(){return new this.constructor(this.array,this.itemSize).copy(this)}toJSON(){const t={itemSize:this.itemSize,type:this.array.constructor.name,array:Array.from(this.array),normalized:this.normalized};return this.name!==""&&(t.name=this.name),this.usage!==hl&&(t.usage=this.usage),t}}class lp extends xn{constructor(t,e,n){super(new Uint16Array(t),e,n)}}class hp extends xn{constructor(t,e,n){super(new Uint32Array(t),e,n)}}class Re extends xn{constructor(t,e,n){super(new Float32Array(t),e,n)}}let B_=0;const Rn=new Be,Lc=new je,Or=new k,wn=new po,Bs=new po,We=new k;class yn extends gr{constructor(){super(),this.isBufferGeometry=!0,Object.defineProperty(this,"id",{value:B_++}),this.uuid=Ui(),this.name="",this.type="BufferGeometry",this.index=null,this.attributes={},this.morphAttributes={},this.morphTargetsRelative=!1,this.groups=[],this.boundingBox=null,this.boundingSphere=null,this.drawRange={start:0,count:1/0},this.userData={}}getIndex(){return this.index}setIndex(t){return Array.isArray(t)?this.index=new(rp(t)?hp:lp)(t,1):this.index=t,this}getAttribute(t){return this.attributes[t]}setAttribute(t,e){return this.attributes[t]=e,this}deleteAttribute(t){return delete this.attributes[t],this}hasAttribute(t){return this.attributes[t]!==void 0}addGroup(t,e,n=0){this.groups.push({start:t,count:e,materialIndex:n})}clearGroups(){this.groups=[]}setDrawRange(t,e){this.drawRange.start=t,this.drawRange.count=e}applyMatrix4(t){const e=this.attributes.position;e!==void 0&&(e.applyMatrix4(t),e.needsUpdate=!0);const n=this.attributes.normal;if(n!==void 0){const s=new ae().getNormalMatrix(t);n.applyNormalMatrix(s),n.needsUpdate=!0}const r=this.attributes.tangent;return r!==void 0&&(r.transformDirection(t),r.needsUpdate=!0),this.boundingBox!==null&&this.computeBoundingBox(),this.boundingSphere!==null&&this.computeBoundingSphere(),this}applyQuaternion(t){return Rn.makeRotationFromQuaternion(t),this.applyMatrix4(Rn),this}rotateX(t){return Rn.makeRotationX(t),this.applyMatrix4(Rn),this}rotateY(t){return Rn.makeRotationY(t),this.applyMatrix4(Rn),this}rotateZ(t){return Rn.makeRotationZ(t),this.applyMatrix4(Rn),this}translate(t,e,n){return Rn.makeTranslation(t,e,n),this.applyMatrix4(Rn),this}scale(t,e,n){return Rn.makeScale(t,e,n),this.applyMatrix4(Rn),this}lookAt(t){return Lc.lookAt(t),Lc.updateMatrix(),this.applyMatrix4(Lc.matrix),this}center(){return this.computeBoundingBox(),this.boundingBox.getCenter(Or).negate(),this.translate(Or.x,Or.y,Or.z),this}setFromPoints(t){const e=[];for(let n=0,r=t.length;n<r;n++){const s=t[n];e.push(s.x,s.y,s.z||0)}return this.setAttribute("position",new Re(e,3)),this}computeBoundingBox(){this.boundingBox===null&&(this.boundingBox=new po);const t=this.attributes.position,e=this.morphAttributes.position;if(t&&t.isGLBufferAttribute){console.error('THREE.BufferGeometry.computeBoundingBox(): GLBufferAttribute requires a manual bounding box. Alternatively set "mesh.frustumCulled" to "false".',this),this.boundingBox.set(new k(-1/0,-1/0,-1/0),new k(1/0,1/0,1/0));return}if(t!==void 0){if(this.boundingBox.setFromBufferAttribute(t),e)for(let n=0,r=e.length;n<r;n++){const s=e[n];wn.setFromBufferAttribute(s),this.morphTargetsRelative?(We.addVectors(this.boundingBox.min,wn.min),this.boundingBox.expandByPoint(We),We.addVectors(this.boundingBox.max,wn.max),this.boundingBox.expandByPoint(We)):(this.boundingBox.expandByPoint(wn.min),this.boundingBox.expandByPoint(wn.max))}}else this.boundingBox.makeEmpty();(isNaN(this.boundingBox.min.x)||isNaN(this.boundingBox.min.y)||isNaN(this.boundingBox.min.z))&&console.error('THREE.BufferGeometry.computeBoundingBox(): Computed min/max have NaN values. The "position" attribute is likely to have NaN values.',this)}computeBoundingSphere(){this.boundingSphere===null&&(this.boundingSphere=new Bl);const t=this.attributes.position,e=this.morphAttributes.position;if(t&&t.isGLBufferAttribute){console.error('THREE.BufferGeometry.computeBoundingSphere(): GLBufferAttribute requires a manual bounding sphere. Alternatively set "mesh.frustumCulled" to "false".',this),this.boundingSphere.set(new k,1/0);return}if(t){const n=this.boundingSphere.center;if(wn.setFromBufferAttribute(t),e)for(let s=0,o=e.length;s<o;s++){const a=e[s];Bs.setFromBufferAttribute(a),this.morphTargetsRelative?(We.addVectors(wn.min,Bs.min),wn.expandByPoint(We),We.addVectors(wn.max,Bs.max),wn.expandByPoint(We)):(wn.expandByPoint(Bs.min),wn.expandByPoint(Bs.max))}wn.getCenter(n);let r=0;for(let s=0,o=t.count;s<o;s++)We.fromBufferAttribute(t,s),r=Math.max(r,n.distanceToSquared(We));if(e)for(let s=0,o=e.length;s<o;s++){const a=e[s],c=this.morphTargetsRelative;for(let h=0,u=a.count;h<u;h++)We.fromBufferAttribute(a,h),c&&(Or.fromBufferAttribute(t,h),We.add(Or)),r=Math.max(r,n.distanceToSquared(We))}this.boundingSphere.radius=Math.sqrt(r),isNaN(this.boundingSphere.radius)&&console.error('THREE.BufferGeometry.computeBoundingSphere(): Computed radius is NaN. The "position" attribute is likely to have NaN values.',this)}}computeTangents(){const t=this.index,e=this.attributes;if(t===null||e.position===void 0||e.normal===void 0||e.uv===void 0){console.error("THREE.BufferGeometry: .computeTangents() failed. Missing required attributes (index, position, normal or uv)");return}const n=t.array,r=e.position.array,s=e.normal.array,o=e.uv.array,a=r.length/3;this.hasAttribute("tangent")===!1&&this.setAttribute("tangent",new xn(new Float32Array(4*a),4));const c=this.getAttribute("tangent").array,h=[],u=[];for(let S=0;S<a;S++)h[S]=new k,u[S]=new k;const d=new k,f=new k,p=new k,y=new Vt,E=new Vt,_=new Vt,m=new k,C=new k;function w(S,P,L){d.fromArray(r,S*3),f.fromArray(r,P*3),p.fromArray(r,L*3),y.fromArray(o,S*2),E.fromArray(o,P*2),_.fromArray(o,L*2),f.sub(d),p.sub(d),E.sub(y),_.sub(y);const R=1/(E.x*_.y-_.x*E.y);isFinite(R)&&(m.copy(f).multiplyScalar(_.y).addScaledVector(p,-E.y).multiplyScalar(R),C.copy(p).multiplyScalar(E.x).addScaledVector(f,-_.x).multiplyScalar(R),h[S].add(m),h[P].add(m),h[L].add(m),u[S].add(C),u[P].add(C),u[L].add(C))}let b=this.groups;b.length===0&&(b=[{start:0,count:n.length}]);for(let S=0,P=b.length;S<P;++S){const L=b[S],R=L.start,V=L.count;for(let G=R,K=R+V;G<K;G+=3)w(n[G+0],n[G+1],n[G+2])}const N=new k,U=new k,T=new k,M=new k;function v(S){T.fromArray(s,S*3),M.copy(T);const P=h[S];N.copy(P),N.sub(T.multiplyScalar(T.dot(P))).normalize(),U.crossVectors(M,P);const R=U.dot(u[S])<0?-1:1;c[S*4]=N.x,c[S*4+1]=N.y,c[S*4+2]=N.z,c[S*4+3]=R}for(let S=0,P=b.length;S<P;++S){const L=b[S],R=L.start,V=L.count;for(let G=R,K=R+V;G<K;G+=3)v(n[G+0]),v(n[G+1]),v(n[G+2])}}computeVertexNormals(){const t=this.index,e=this.getAttribute("position");if(e!==void 0){let n=this.getAttribute("normal");if(n===void 0)n=new xn(new Float32Array(e.count*3),3),this.setAttribute("normal",n);else for(let f=0,p=n.count;f<p;f++)n.setXYZ(f,0,0,0);const r=new k,s=new k,o=new k,a=new k,c=new k,h=new k,u=new k,d=new k;if(t)for(let f=0,p=t.count;f<p;f+=3){const y=t.getX(f+0),E=t.getX(f+1),_=t.getX(f+2);r.fromBufferAttribute(e,y),s.fromBufferAttribute(e,E),o.fromBufferAttribute(e,_),u.subVectors(o,s),d.subVectors(r,s),u.cross(d),a.fromBufferAttribute(n,y),c.fromBufferAttribute(n,E),h.fromBufferAttribute(n,_),a.add(u),c.add(u),h.add(u),n.setXYZ(y,a.x,a.y,a.z),n.setXYZ(E,c.x,c.y,c.z),n.setXYZ(_,h.x,h.y,h.z)}else for(let f=0,p=e.count;f<p;f+=3)r.fromBufferAttribute(e,f+0),s.fromBufferAttribute(e,f+1),o.fromBufferAttribute(e,f+2),u.subVectors(o,s),d.subVectors(r,s),u.cross(d),n.setXYZ(f+0,u.x,u.y,u.z),n.setXYZ(f+1,u.x,u.y,u.z),n.setXYZ(f+2,u.x,u.y,u.z);this.normalizeNormals(),n.needsUpdate=!0}}normalizeNormals(){const t=this.attributes.normal;for(let e=0,n=t.count;e<n;e++)We.fromBufferAttribute(t,e),We.normalize(),t.setXYZ(e,We.x,We.y,We.z)}toNonIndexed(){function t(a,c){const h=a.array,u=a.itemSize,d=a.normalized,f=new h.constructor(c.length*u);let p=0,y=0;for(let E=0,_=c.length;E<_;E++){a.isInterleavedBufferAttribute?p=c[E]*a.data.stride+a.offset:p=c[E]*u;for(let m=0;m<u;m++)f[y++]=h[p++]}return new xn(f,u,d)}if(this.index===null)return console.warn("THREE.BufferGeometry.toNonIndexed(): BufferGeometry is already non-indexed."),this;const e=new yn,n=this.index.array,r=this.attributes;for(const a in r){const c=r[a],h=t(c,n);e.setAttribute(a,h)}const s=this.morphAttributes;for(const a in s){const c=[],h=s[a];for(let u=0,d=h.length;u<d;u++){const f=h[u],p=t(f,n);c.push(p)}e.morphAttributes[a]=c}e.morphTargetsRelative=this.morphTargetsRelative;const o=this.groups;for(let a=0,c=o.length;a<c;a++){const h=o[a];e.addGroup(h.start,h.count,h.materialIndex)}return e}toJSON(){const t={metadata:{version:4.6,type:"BufferGeometry",generator:"BufferGeometry.toJSON"}};if(t.uuid=this.uuid,t.type=this.type,this.name!==""&&(t.name=this.name),Object.keys(this.userData).length>0&&(t.userData=this.userData),this.parameters!==void 0){const c=this.parameters;for(const h in c)c[h]!==void 0&&(t[h]=c[h]);return t}t.data={attributes:{}};const e=this.index;e!==null&&(t.data.index={type:e.array.constructor.name,array:Array.prototype.slice.call(e.array)});const n=this.attributes;for(const c in n){const h=n[c];t.data.attributes[c]=h.toJSON(t.data)}const r={};let s=!1;for(const c in this.morphAttributes){const h=this.morphAttributes[c],u=[];for(let d=0,f=h.length;d<f;d++){const p=h[d];u.push(p.toJSON(t.data))}u.length>0&&(r[c]=u,s=!0)}s&&(t.data.morphAttributes=r,t.data.morphTargetsRelative=this.morphTargetsRelative);const o=this.groups;o.length>0&&(t.data.groups=JSON.parse(JSON.stringify(o)));const a=this.boundingSphere;return a!==null&&(t.data.boundingSphere={center:a.center.toArray(),radius:a.radius}),t}clone(){return new this.constructor().copy(this)}copy(t){this.index=null,this.attributes={},this.morphAttributes={},this.groups=[],this.boundingBox=null,this.boundingSphere=null;const e={};this.name=t.name;const n=t.index;n!==null&&this.setIndex(n.clone(e));const r=t.attributes;for(const h in r){const u=r[h];this.setAttribute(h,u.clone(e))}const s=t.morphAttributes;for(const h in s){const u=[],d=s[h];for(let f=0,p=d.length;f<p;f++)u.push(d[f].clone(e));this.morphAttributes[h]=u}this.morphTargetsRelative=t.morphTargetsRelative;const o=t.groups;for(let h=0,u=o.length;h<u;h++){const d=o[h];this.addGroup(d.start,d.count,d.materialIndex)}const a=t.boundingBox;a!==null&&(this.boundingBox=a.clone());const c=t.boundingSphere;return c!==null&&(this.boundingSphere=c.clone()),this.drawRange.start=t.drawRange.start,this.drawRange.count=t.drawRange.count,this.userData=t.userData,this}dispose(){this.dispatchEvent({type:"dispose"})}}const Ku=new Be,Qi=new kl,Wo=new Bl,Qu=new k,Fr=new k,Vr=new k,Br=new k,Nc=new k,qo=new k,Xo=new Vt,jo=new Vt,Yo=new Vt,Zu=new k,Ju=new k,td=new k,$o=new k,Ko=new k;class Ie extends je{constructor(t=new yn,e=new bn){super(),this.isMesh=!0,this.type="Mesh",this.geometry=t,this.material=e,this.updateMorphTargets()}copy(t,e){return super.copy(t,e),t.morphTargetInfluences!==void 0&&(this.morphTargetInfluences=t.morphTargetInfluences.slice()),t.morphTargetDictionary!==void 0&&(this.morphTargetDictionary=Object.assign({},t.morphTargetDictionary)),this.material=Array.isArray(t.material)?t.material.slice():t.material,this.geometry=t.geometry,this}updateMorphTargets(){const e=this.geometry.morphAttributes,n=Object.keys(e);if(n.length>0){const r=e[n[0]];if(r!==void 0){this.morphTargetInfluences=[],this.morphTargetDictionary={};for(let s=0,o=r.length;s<o;s++){const a=r[s].name||String(s);this.morphTargetInfluences.push(0),this.morphTargetDictionary[a]=s}}}}getVertexPosition(t,e){const n=this.geometry,r=n.attributes.position,s=n.morphAttributes.position,o=n.morphTargetsRelative;e.fromBufferAttribute(r,t);const a=this.morphTargetInfluences;if(s&&a){qo.set(0,0,0);for(let c=0,h=s.length;c<h;c++){const u=a[c],d=s[c];u!==0&&(Nc.fromBufferAttribute(d,t),o?qo.addScaledVector(Nc,u):qo.addScaledVector(Nc.sub(e),u))}e.add(qo)}return e}raycast(t,e){const n=this.geometry,r=this.material,s=this.matrixWorld;r!==void 0&&(n.boundingSphere===null&&n.computeBoundingSphere(),Wo.copy(n.boundingSphere),Wo.applyMatrix4(s),Qi.copy(t.ray).recast(t.near),!(Wo.containsPoint(Qi.origin)===!1&&(Qi.intersectSphere(Wo,Qu)===null||Qi.origin.distanceToSquared(Qu)>(t.far-t.near)**2))&&(Ku.copy(s).invert(),Qi.copy(t.ray).applyMatrix4(Ku),!(n.boundingBox!==null&&Qi.intersectsBox(n.boundingBox)===!1)&&this._computeIntersections(t,e,Qi)))}_computeIntersections(t,e,n){let r;const s=this.geometry,o=this.material,a=s.index,c=s.attributes.position,h=s.attributes.uv,u=s.attributes.uv1,d=s.attributes.normal,f=s.groups,p=s.drawRange;if(a!==null)if(Array.isArray(o))for(let y=0,E=f.length;y<E;y++){const _=f[y],m=o[_.materialIndex],C=Math.max(_.start,p.start),w=Math.min(a.count,Math.min(_.start+_.count,p.start+p.count));for(let b=C,N=w;b<N;b+=3){const U=a.getX(b),T=a.getX(b+1),M=a.getX(b+2);r=Qo(this,m,t,n,h,u,d,U,T,M),r&&(r.faceIndex=Math.floor(b/3),r.face.materialIndex=_.materialIndex,e.push(r))}}else{const y=Math.max(0,p.start),E=Math.min(a.count,p.start+p.count);for(let _=y,m=E;_<m;_+=3){const C=a.getX(_),w=a.getX(_+1),b=a.getX(_+2);r=Qo(this,o,t,n,h,u,d,C,w,b),r&&(r.faceIndex=Math.floor(_/3),e.push(r))}}else if(c!==void 0)if(Array.isArray(o))for(let y=0,E=f.length;y<E;y++){const _=f[y],m=o[_.materialIndex],C=Math.max(_.start,p.start),w=Math.min(c.count,Math.min(_.start+_.count,p.start+p.count));for(let b=C,N=w;b<N;b+=3){const U=b,T=b+1,M=b+2;r=Qo(this,m,t,n,h,u,d,U,T,M),r&&(r.faceIndex=Math.floor(b/3),r.face.materialIndex=_.materialIndex,e.push(r))}}else{const y=Math.max(0,p.start),E=Math.min(c.count,p.start+p.count);for(let _=y,m=E;_<m;_+=3){const C=_,w=_+1,b=_+2;r=Qo(this,o,t,n,h,u,d,C,w,b),r&&(r.faceIndex=Math.floor(_/3),e.push(r))}}}}function k_(i,t,e,n,r,s,o,a){let c;if(t.side===vn?c=n.intersectTriangle(o,s,r,!0,a):c=n.intersectTriangle(r,s,o,t.side===ui,a),c===null)return null;Ko.copy(a),Ko.applyMatrix4(i.matrixWorld);const h=e.ray.origin.distanceTo(Ko);return h<e.near||h>e.far?null:{distance:h,point:Ko.clone(),object:i}}function Qo(i,t,e,n,r,s,o,a,c,h){i.getVertexPosition(a,Fr),i.getVertexPosition(c,Vr),i.getVertexPosition(h,Br);const u=k_(i,t,e,n,Fr,Vr,Br,$o);if(u){r&&(Xo.fromBufferAttribute(r,a),jo.fromBufferAttribute(r,c),Yo.fromBufferAttribute(r,h),u.uv=Dn.getInterpolation($o,Fr,Vr,Br,Xo,jo,Yo,new Vt)),s&&(Xo.fromBufferAttribute(s,a),jo.fromBufferAttribute(s,c),Yo.fromBufferAttribute(s,h),u.uv1=Dn.getInterpolation($o,Fr,Vr,Br,Xo,jo,Yo,new Vt),u.uv2=u.uv1),o&&(Zu.fromBufferAttribute(o,a),Ju.fromBufferAttribute(o,c),td.fromBufferAttribute(o,h),u.normal=Dn.getInterpolation($o,Fr,Vr,Br,Zu,Ju,td,new k),u.normal.dot(n.direction)>0&&u.normal.multiplyScalar(-1));const d={a,b:c,c:h,normal:new k,materialIndex:0};Dn.getNormal(Fr,Vr,Br,d.normal),u.face=d}return u}class _s extends yn{constructor(t=1,e=1,n=1,r=1,s=1,o=1){super(),this.type="BoxGeometry",this.parameters={width:t,height:e,depth:n,widthSegments:r,heightSegments:s,depthSegments:o};const a=this;r=Math.floor(r),s=Math.floor(s),o=Math.floor(o);const c=[],h=[],u=[],d=[];let f=0,p=0;y("z","y","x",-1,-1,n,e,t,o,s,0),y("z","y","x",1,-1,n,e,-t,o,s,1),y("x","z","y",1,1,t,n,e,r,o,2),y("x","z","y",1,-1,t,n,-e,r,o,3),y("x","y","z",1,-1,t,e,n,r,s,4),y("x","y","z",-1,-1,t,e,-n,r,s,5),this.setIndex(c),this.setAttribute("position",new Re(h,3)),this.setAttribute("normal",new Re(u,3)),this.setAttribute("uv",new Re(d,2));function y(E,_,m,C,w,b,N,U,T,M,v){const S=b/T,P=N/M,L=b/2,R=N/2,V=U/2,G=T+1,K=M+1;let Z=0,J=0;const et=new k;for(let tt=0;tt<K;tt++){const pt=tt*P-R;for(let ht=0;ht<G;ht++){const X=ht*S-L;et[E]=X*C,et[_]=pt*w,et[m]=V,h.push(et.x,et.y,et.z),et[E]=0,et[_]=0,et[m]=U>0?1:-1,u.push(et.x,et.y,et.z),d.push(ht/T),d.push(1-tt/M),Z+=1}}for(let tt=0;tt<M;tt++)for(let pt=0;pt<T;pt++){const ht=f+pt+G*tt,X=f+pt+G*(tt+1),it=f+(pt+1)+G*(tt+1),mt=f+(pt+1)+G*tt;c.push(ht,X,mt),c.push(X,it,mt),J+=6}a.addGroup(p,J,v),p+=J,f+=Z}}copy(t){return super.copy(t),this.parameters=Object.assign({},t.parameters),this}static fromJSON(t){return new _s(t.width,t.height,t.depth,t.widthSegments,t.heightSegments,t.depthSegments)}}function as(i){const t={};for(const e in i){t[e]={};for(const n in i[e]){const r=i[e][n];r&&(r.isColor||r.isMatrix3||r.isMatrix4||r.isVector2||r.isVector3||r.isVector4||r.isTexture||r.isQuaternion)?r.isRenderTargetTexture?(console.warn("UniformsUtils: Textures of render targets cannot be cloned via cloneUniforms() or mergeUniforms()."),t[e][n]=null):t[e][n]=r.clone():Array.isArray(r)?t[e][n]=r.slice():t[e][n]=r}}return t}function fn(i){const t={};for(let e=0;e<i.length;e++){const n=as(i[e]);for(const r in n)t[r]=n[r]}return t}function H_(i){const t=[];for(let e=0;e<i.length;e++)t.push(i[e].clone());return t}function up(i){return i.getRenderTarget()===null?i.outputColorSpace:ve.workingColorSpace}const z_={clone:as,merge:fn};var G_=`void main() {
	gl_Position = projectionMatrix * modelViewMatrix * vec4( position, 1.0 );
}`,W_=`void main() {
	gl_FragColor = vec4( 1.0, 0.0, 0.0, 1.0 );
}`;class fi extends gs{constructor(t){super(),this.isShaderMaterial=!0,this.type="ShaderMaterial",this.defines={},this.uniforms={},this.uniformsGroups=[],this.vertexShader=G_,this.fragmentShader=W_,this.linewidth=1,this.wireframe=!1,this.wireframeLinewidth=1,this.fog=!1,this.lights=!1,this.clipping=!1,this.forceSinglePass=!0,this.extensions={derivatives:!1,fragDepth:!1,drawBuffers:!1,shaderTextureLOD:!1,clipCullDistance:!1},this.defaultAttributeValues={color:[1,1,1],uv:[0,0],uv1:[0,0]},this.index0AttributeName=void 0,this.uniformsNeedUpdate=!1,this.glslVersion=null,t!==void 0&&this.setValues(t)}copy(t){return super.copy(t),this.fragmentShader=t.fragmentShader,this.vertexShader=t.vertexShader,this.uniforms=as(t.uniforms),this.uniformsGroups=H_(t.uniformsGroups),this.defines=Object.assign({},t.defines),this.wireframe=t.wireframe,this.wireframeLinewidth=t.wireframeLinewidth,this.fog=t.fog,this.lights=t.lights,this.clipping=t.clipping,this.extensions=Object.assign({},t.extensions),this.glslVersion=t.glslVersion,this}toJSON(t){const e=super.toJSON(t);e.glslVersion=this.glslVersion,e.uniforms={};for(const r in this.uniforms){const o=this.uniforms[r].value;o&&o.isTexture?e.uniforms[r]={type:"t",value:o.toJSON(t).uuid}:o&&o.isColor?e.uniforms[r]={type:"c",value:o.getHex()}:o&&o.isVector2?e.uniforms[r]={type:"v2",value:o.toArray()}:o&&o.isVector3?e.uniforms[r]={type:"v3",value:o.toArray()}:o&&o.isVector4?e.uniforms[r]={type:"v4",value:o.toArray()}:o&&o.isMatrix3?e.uniforms[r]={type:"m3",value:o.toArray()}:o&&o.isMatrix4?e.uniforms[r]={type:"m4",value:o.toArray()}:e.uniforms[r]={value:o}}Object.keys(this.defines).length>0&&(e.defines=this.defines),e.vertexShader=this.vertexShader,e.fragmentShader=this.fragmentShader,e.lights=this.lights,e.clipping=this.clipping;const n={};for(const r in this.extensions)this.extensions[r]===!0&&(n[r]=!0);return Object.keys(n).length>0&&(e.extensions=n),e}}class dp extends je{constructor(){super(),this.isCamera=!0,this.type="Camera",this.matrixWorldInverse=new Be,this.projectionMatrix=new Be,this.projectionMatrixInverse=new Be,this.coordinateSystem=li}copy(t,e){return super.copy(t,e),this.matrixWorldInverse.copy(t.matrixWorldInverse),this.projectionMatrix.copy(t.projectionMatrix),this.projectionMatrixInverse.copy(t.projectionMatrixInverse),this.coordinateSystem=t.coordinateSystem,this}getWorldDirection(t){return super.getWorldDirection(t).negate()}updateMatrixWorld(t){super.updateMatrixWorld(t),this.matrixWorldInverse.copy(this.matrixWorld).invert()}updateWorldMatrix(t,e){super.updateWorldMatrix(t,e),this.matrixWorldInverse.copy(this.matrixWorld).invert()}clone(){return new this.constructor().copy(this)}}class Ln extends dp{constructor(t=50,e=1,n=.1,r=2e3){super(),this.isPerspectiveCamera=!0,this.type="PerspectiveCamera",this.fov=t,this.zoom=1,this.near=n,this.far=r,this.focus=10,this.aspect=e,this.view=null,this.filmGauge=35,this.filmOffset=0,this.updateProjectionMatrix()}copy(t,e){return super.copy(t,e),this.fov=t.fov,this.zoom=t.zoom,this.near=t.near,this.far=t.far,this.focus=t.focus,this.aspect=t.aspect,this.view=t.view===null?null:Object.assign({},t.view),this.filmGauge=t.filmGauge,this.filmOffset=t.filmOffset,this}setFocalLength(t){const e=.5*this.getFilmHeight()/t;this.fov=dl*2*Math.atan(e),this.updateProjectionMatrix()}getFocalLength(){const t=Math.tan(fa*.5*this.fov);return .5*this.getFilmHeight()/t}getEffectiveFOV(){return dl*2*Math.atan(Math.tan(fa*.5*this.fov)/this.zoom)}getFilmWidth(){return this.filmGauge*Math.min(this.aspect,1)}getFilmHeight(){return this.filmGauge/Math.max(this.aspect,1)}setViewOffset(t,e,n,r,s,o){this.aspect=t/e,this.view===null&&(this.view={enabled:!0,fullWidth:1,fullHeight:1,offsetX:0,offsetY:0,width:1,height:1}),this.view.enabled=!0,this.view.fullWidth=t,this.view.fullHeight=e,this.view.offsetX=n,this.view.offsetY=r,this.view.width=s,this.view.height=o,this.updateProjectionMatrix()}clearViewOffset(){this.view!==null&&(this.view.enabled=!1),this.updateProjectionMatrix()}updateProjectionMatrix(){const t=this.near;let e=t*Math.tan(fa*.5*this.fov)/this.zoom,n=2*e,r=this.aspect*n,s=-.5*r;const o=this.view;if(this.view!==null&&this.view.enabled){const c=o.fullWidth,h=o.fullHeight;s+=o.offsetX*r/c,e-=o.offsetY*n/h,r*=o.width/c,n*=o.height/h}const a=this.filmOffset;a!==0&&(s+=t*a/this.getFilmWidth()),this.projectionMatrix.makePerspective(s,s+r,e,e-n,t,this.far,this.coordinateSystem),this.projectionMatrixInverse.copy(this.projectionMatrix).invert()}toJSON(t){const e=super.toJSON(t);return e.object.fov=this.fov,e.object.zoom=this.zoom,e.object.near=this.near,e.object.far=this.far,e.object.focus=this.focus,e.object.aspect=this.aspect,this.view!==null&&(e.object.view=Object.assign({},this.view)),e.object.filmGauge=this.filmGauge,e.object.filmOffset=this.filmOffset,e}}const kr=-90,Hr=1;class q_ extends je{constructor(t,e,n){super(),this.type="CubeCamera",this.renderTarget=n,this.coordinateSystem=null,this.activeMipmapLevel=0;const r=new Ln(kr,Hr,t,e);r.layers=this.layers,this.add(r);const s=new Ln(kr,Hr,t,e);s.layers=this.layers,this.add(s);const o=new Ln(kr,Hr,t,e);o.layers=this.layers,this.add(o);const a=new Ln(kr,Hr,t,e);a.layers=this.layers,this.add(a);const c=new Ln(kr,Hr,t,e);c.layers=this.layers,this.add(c);const h=new Ln(kr,Hr,t,e);h.layers=this.layers,this.add(h)}updateCoordinateSystem(){const t=this.coordinateSystem,e=this.children.concat(),[n,r,s,o,a,c]=e;for(const h of e)this.remove(h);if(t===li)n.up.set(0,1,0),n.lookAt(1,0,0),r.up.set(0,1,0),r.lookAt(-1,0,0),s.up.set(0,0,-1),s.lookAt(0,1,0),o.up.set(0,0,1),o.lookAt(0,-1,0),a.up.set(0,1,0),a.lookAt(0,0,1),c.up.set(0,1,0),c.lookAt(0,0,-1);else if(t===wa)n.up.set(0,-1,0),n.lookAt(-1,0,0),r.up.set(0,-1,0),r.lookAt(1,0,0),s.up.set(0,0,1),s.lookAt(0,1,0),o.up.set(0,0,-1),o.lookAt(0,-1,0),a.up.set(0,-1,0),a.lookAt(0,0,1),c.up.set(0,-1,0),c.lookAt(0,0,-1);else throw new Error("THREE.CubeCamera.updateCoordinateSystem(): Invalid coordinate system: "+t);for(const h of e)this.add(h),h.updateMatrixWorld()}update(t,e){this.parent===null&&this.updateMatrixWorld();const{renderTarget:n,activeMipmapLevel:r}=this;this.coordinateSystem!==t.coordinateSystem&&(this.coordinateSystem=t.coordinateSystem,this.updateCoordinateSystem());const[s,o,a,c,h,u]=this.children,d=t.getRenderTarget(),f=t.getActiveCubeFace(),p=t.getActiveMipmapLevel(),y=t.xr.enabled;t.xr.enabled=!1;const E=n.texture.generateMipmaps;n.texture.generateMipmaps=!1,t.setRenderTarget(n,0,r),t.render(e,s),t.setRenderTarget(n,1,r),t.render(e,o),t.setRenderTarget(n,2,r),t.render(e,a),t.setRenderTarget(n,3,r),t.render(e,c),t.setRenderTarget(n,4,r),t.render(e,h),n.texture.generateMipmaps=E,t.setRenderTarget(n,5,r),t.render(e,u),t.setRenderTarget(d,f,p),t.xr.enabled=y,n.texture.needsPMREMUpdate=!0}}class fp extends En{constructor(t,e,n,r,s,o,a,c,h,u){t=t!==void 0?t:[],e=e!==void 0?e:rs,super(t,e,n,r,s,o,a,c,h,u),this.isCubeTexture=!0,this.flipY=!1}get images(){return this.image}set images(t){this.image=t}}class X_ extends hr{constructor(t=1,e={}){super(t,t,e),this.isWebGLCubeRenderTarget=!0;const n={width:t,height:t,depth:1},r=[n,n,n,n,n,n];e.encoding!==void 0&&(Ys("THREE.WebGLCubeRenderTarget: option.encoding has been replaced by option.colorSpace."),e.colorSpace=e.encoding===cr?Ze:Nn),this.texture=new fp(r,e.mapping,e.wrapS,e.wrapT,e.magFilter,e.minFilter,e.format,e.type,e.anisotropy,e.colorSpace),this.texture.isRenderTargetTexture=!0,this.texture.generateMipmaps=e.generateMipmaps!==void 0?e.generateMipmaps:!1,this.texture.minFilter=e.minFilter!==void 0?e.minFilter:Pn}fromEquirectangularTexture(t,e){this.texture.type=e.type,this.texture.colorSpace=e.colorSpace,this.texture.generateMipmaps=e.generateMipmaps,this.texture.minFilter=e.minFilter,this.texture.magFilter=e.magFilter;const n={uniforms:{tEquirect:{value:null}},vertexShader:`

				varying vec3 vWorldDirection;

				vec3 transformDirection( in vec3 dir, in mat4 matrix ) {

					return normalize( ( matrix * vec4( dir, 0.0 ) ).xyz );

				}

				void main() {

					vWorldDirection = transformDirection( position, modelMatrix );

					#include <begin_vertex>
					#include <project_vertex>

				}
			`,fragmentShader:`

				uniform sampler2D tEquirect;

				varying vec3 vWorldDirection;

				#include <common>

				void main() {

					vec3 direction = normalize( vWorldDirection );

					vec2 sampleUV = equirectUv( direction );

					gl_FragColor = texture2D( tEquirect, sampleUV );

				}
			`},r=new _s(5,5,5),s=new fi({name:"CubemapFromEquirect",uniforms:as(n.uniforms),vertexShader:n.vertexShader,fragmentShader:n.fragmentShader,side:vn,blending:Di});s.uniforms.tEquirect.value=e;const o=new Ie(r,s),a=e.minFilter;return e.minFilter===to&&(e.minFilter=Pn),new q_(1,10,this).update(t,o),e.minFilter=a,o.geometry.dispose(),o.material.dispose(),this}clear(t,e,n,r){const s=t.getRenderTarget();for(let o=0;o<6;o++)t.setRenderTarget(this,o),t.clear(e,n,r);t.setRenderTarget(s)}}const Uc=new k,j_=new k,Y_=new ae;class bi{constructor(t=new k(1,0,0),e=0){this.isPlane=!0,this.normal=t,this.constant=e}set(t,e){return this.normal.copy(t),this.constant=e,this}setComponents(t,e,n,r){return this.normal.set(t,e,n),this.constant=r,this}setFromNormalAndCoplanarPoint(t,e){return this.normal.copy(t),this.constant=-e.dot(this.normal),this}setFromCoplanarPoints(t,e,n){const r=Uc.subVectors(n,e).cross(j_.subVectors(t,e)).normalize();return this.setFromNormalAndCoplanarPoint(r,t),this}copy(t){return this.normal.copy(t.normal),this.constant=t.constant,this}normalize(){const t=1/this.normal.length();return this.normal.multiplyScalar(t),this.constant*=t,this}negate(){return this.constant*=-1,this.normal.negate(),this}distanceToPoint(t){return this.normal.dot(t)+this.constant}distanceToSphere(t){return this.distanceToPoint(t.center)-t.radius}projectPoint(t,e){return e.copy(t).addScaledVector(this.normal,-this.distanceToPoint(t))}intersectLine(t,e){const n=t.delta(Uc),r=this.normal.dot(n);if(r===0)return this.distanceToPoint(t.start)===0?e.copy(t.start):null;const s=-(t.start.dot(this.normal)+this.constant)/r;return s<0||s>1?null:e.copy(t.start).addScaledVector(n,s)}intersectsLine(t){const e=this.distanceToPoint(t.start),n=this.distanceToPoint(t.end);return e<0&&n>0||n<0&&e>0}intersectsBox(t){return t.intersectsPlane(this)}intersectsSphere(t){return t.intersectsPlane(this)}coplanarPoint(t){return t.copy(this.normal).multiplyScalar(-this.constant)}applyMatrix4(t,e){const n=e||Y_.getNormalMatrix(t),r=this.coplanarPoint(Uc).applyMatrix4(t),s=this.normal.applyMatrix3(n).normalize();return this.constant=-r.dot(s),this}translate(t){return this.constant-=t.dot(this.normal),this}equals(t){return t.normal.equals(this.normal)&&t.constant===this.constant}clone(){return new this.constructor().copy(this)}}const Zi=new Bl,Zo=new k;class zl{constructor(t=new bi,e=new bi,n=new bi,r=new bi,s=new bi,o=new bi){this.planes=[t,e,n,r,s,o]}set(t,e,n,r,s,o){const a=this.planes;return a[0].copy(t),a[1].copy(e),a[2].copy(n),a[3].copy(r),a[4].copy(s),a[5].copy(o),this}copy(t){const e=this.planes;for(let n=0;n<6;n++)e[n].copy(t.planes[n]);return this}setFromProjectionMatrix(t,e=li){const n=this.planes,r=t.elements,s=r[0],o=r[1],a=r[2],c=r[3],h=r[4],u=r[5],d=r[6],f=r[7],p=r[8],y=r[9],E=r[10],_=r[11],m=r[12],C=r[13],w=r[14],b=r[15];if(n[0].setComponents(c-s,f-h,_-p,b-m).normalize(),n[1].setComponents(c+s,f+h,_+p,b+m).normalize(),n[2].setComponents(c+o,f+u,_+y,b+C).normalize(),n[3].setComponents(c-o,f-u,_-y,b-C).normalize(),n[4].setComponents(c-a,f-d,_-E,b-w).normalize(),e===li)n[5].setComponents(c+a,f+d,_+E,b+w).normalize();else if(e===wa)n[5].setComponents(a,d,E,w).normalize();else throw new Error("THREE.Frustum.setFromProjectionMatrix(): Invalid coordinate system: "+e);return this}intersectsObject(t){if(t.boundingSphere!==void 0)t.boundingSphere===null&&t.computeBoundingSphere(),Zi.copy(t.boundingSphere).applyMatrix4(t.matrixWorld);else{const e=t.geometry;e.boundingSphere===null&&e.computeBoundingSphere(),Zi.copy(e.boundingSphere).applyMatrix4(t.matrixWorld)}return this.intersectsSphere(Zi)}intersectsSprite(t){return Zi.center.set(0,0,0),Zi.radius=.7071067811865476,Zi.applyMatrix4(t.matrixWorld),this.intersectsSphere(Zi)}intersectsSphere(t){const e=this.planes,n=t.center,r=-t.radius;for(let s=0;s<6;s++)if(e[s].distanceToPoint(n)<r)return!1;return!0}intersectsBox(t){const e=this.planes;for(let n=0;n<6;n++){const r=e[n];if(Zo.x=r.normal.x>0?t.max.x:t.min.x,Zo.y=r.normal.y>0?t.max.y:t.min.y,Zo.z=r.normal.z>0?t.max.z:t.min.z,r.distanceToPoint(Zo)<0)return!1}return!0}containsPoint(t){const e=this.planes;for(let n=0;n<6;n++)if(e[n].distanceToPoint(t)<0)return!1;return!0}clone(){return new this.constructor().copy(this)}}function pp(){let i=null,t=!1,e=null,n=null;function r(s,o){e(s,o),n=i.requestAnimationFrame(r)}return{start:function(){t!==!0&&e!==null&&(n=i.requestAnimationFrame(r),t=!0)},stop:function(){i.cancelAnimationFrame(n),t=!1},setAnimationLoop:function(s){e=s},setContext:function(s){i=s}}}function $_(i,t){const e=t.isWebGL2,n=new WeakMap;function r(h,u){const d=h.array,f=h.usage,p=d.byteLength,y=i.createBuffer();i.bindBuffer(u,y),i.bufferData(u,d,f),h.onUploadCallback();let E;if(d instanceof Float32Array)E=i.FLOAT;else if(d instanceof Uint16Array)if(h.isFloat16BufferAttribute)if(e)E=i.HALF_FLOAT;else throw new Error("THREE.WebGLAttributes: Usage of Float16BufferAttribute requires WebGL2.");else E=i.UNSIGNED_SHORT;else if(d instanceof Int16Array)E=i.SHORT;else if(d instanceof Uint32Array)E=i.UNSIGNED_INT;else if(d instanceof Int32Array)E=i.INT;else if(d instanceof Int8Array)E=i.BYTE;else if(d instanceof Uint8Array)E=i.UNSIGNED_BYTE;else if(d instanceof Uint8ClampedArray)E=i.UNSIGNED_BYTE;else throw new Error("THREE.WebGLAttributes: Unsupported buffer data format: "+d);return{buffer:y,type:E,bytesPerElement:d.BYTES_PER_ELEMENT,version:h.version,size:p}}function s(h,u,d){const f=u.array,p=u._updateRange,y=u.updateRanges;if(i.bindBuffer(d,h),p.count===-1&&y.length===0&&i.bufferSubData(d,0,f),y.length!==0){for(let E=0,_=y.length;E<_;E++){const m=y[E];e?i.bufferSubData(d,m.start*f.BYTES_PER_ELEMENT,f,m.start,m.count):i.bufferSubData(d,m.start*f.BYTES_PER_ELEMENT,f.subarray(m.start,m.start+m.count))}u.clearUpdateRanges()}p.count!==-1&&(e?i.bufferSubData(d,p.offset*f.BYTES_PER_ELEMENT,f,p.offset,p.count):i.bufferSubData(d,p.offset*f.BYTES_PER_ELEMENT,f.subarray(p.offset,p.offset+p.count)),p.count=-1),u.onUploadCallback()}function o(h){return h.isInterleavedBufferAttribute&&(h=h.data),n.get(h)}function a(h){h.isInterleavedBufferAttribute&&(h=h.data);const u=n.get(h);u&&(i.deleteBuffer(u.buffer),n.delete(h))}function c(h,u){if(h.isGLBufferAttribute){const f=n.get(h);(!f||f.version<h.version)&&n.set(h,{buffer:h.buffer,type:h.type,bytesPerElement:h.elementSize,version:h.version});return}h.isInterleavedBufferAttribute&&(h=h.data);const d=n.get(h);if(d===void 0)n.set(h,r(h,u));else if(d.version<h.version){if(d.size!==h.array.byteLength)throw new Error("THREE.WebGLAttributes: The size of the buffer attribute's array buffer does not match the original size. Resizing buffer attributes is not supported.");s(d.buffer,h,u),d.version=h.version}}return{get:o,remove:a,update:c}}class dr extends yn{constructor(t=1,e=1,n=1,r=1){super(),this.type="PlaneGeometry",this.parameters={width:t,height:e,widthSegments:n,heightSegments:r};const s=t/2,o=e/2,a=Math.floor(n),c=Math.floor(r),h=a+1,u=c+1,d=t/a,f=e/c,p=[],y=[],E=[],_=[];for(let m=0;m<u;m++){const C=m*f-o;for(let w=0;w<h;w++){const b=w*d-s;y.push(b,-C,0),E.push(0,0,1),_.push(w/a),_.push(1-m/c)}}for(let m=0;m<c;m++)for(let C=0;C<a;C++){const w=C+h*m,b=C+h*(m+1),N=C+1+h*(m+1),U=C+1+h*m;p.push(w,b,U),p.push(b,N,U)}this.setIndex(p),this.setAttribute("position",new Re(y,3)),this.setAttribute("normal",new Re(E,3)),this.setAttribute("uv",new Re(_,2))}copy(t){return super.copy(t),this.parameters=Object.assign({},t.parameters),this}static fromJSON(t){return new dr(t.width,t.height,t.widthSegments,t.heightSegments)}}var K_=`#ifdef USE_ALPHAHASH
	if ( diffuseColor.a < getAlphaHashThreshold( vPosition ) ) discard;
#endif`,Q_=`#ifdef USE_ALPHAHASH
	const float ALPHA_HASH_SCALE = 0.05;
	float hash2D( vec2 value ) {
		return fract( 1.0e4 * sin( 17.0 * value.x + 0.1 * value.y ) * ( 0.1 + abs( sin( 13.0 * value.y + value.x ) ) ) );
	}
	float hash3D( vec3 value ) {
		return hash2D( vec2( hash2D( value.xy ), value.z ) );
	}
	float getAlphaHashThreshold( vec3 position ) {
		float maxDeriv = max(
			length( dFdx( position.xyz ) ),
			length( dFdy( position.xyz ) )
		);
		float pixScale = 1.0 / ( ALPHA_HASH_SCALE * maxDeriv );
		vec2 pixScales = vec2(
			exp2( floor( log2( pixScale ) ) ),
			exp2( ceil( log2( pixScale ) ) )
		);
		vec2 alpha = vec2(
			hash3D( floor( pixScales.x * position.xyz ) ),
			hash3D( floor( pixScales.y * position.xyz ) )
		);
		float lerpFactor = fract( log2( pixScale ) );
		float x = ( 1.0 - lerpFactor ) * alpha.x + lerpFactor * alpha.y;
		float a = min( lerpFactor, 1.0 - lerpFactor );
		vec3 cases = vec3(
			x * x / ( 2.0 * a * ( 1.0 - a ) ),
			( x - 0.5 * a ) / ( 1.0 - a ),
			1.0 - ( ( 1.0 - x ) * ( 1.0 - x ) / ( 2.0 * a * ( 1.0 - a ) ) )
		);
		float threshold = ( x < ( 1.0 - a ) )
			? ( ( x < a ) ? cases.x : cases.y )
			: cases.z;
		return clamp( threshold , 1.0e-6, 1.0 );
	}
#endif`,Z_=`#ifdef USE_ALPHAMAP
	diffuseColor.a *= texture2D( alphaMap, vAlphaMapUv ).g;
#endif`,J_=`#ifdef USE_ALPHAMAP
	uniform sampler2D alphaMap;
#endif`,tv=`#ifdef USE_ALPHATEST
	if ( diffuseColor.a < alphaTest ) discard;
#endif`,ev=`#ifdef USE_ALPHATEST
	uniform float alphaTest;
#endif`,nv=`#ifdef USE_AOMAP
	float ambientOcclusion = ( texture2D( aoMap, vAoMapUv ).r - 1.0 ) * aoMapIntensity + 1.0;
	reflectedLight.indirectDiffuse *= ambientOcclusion;
	#if defined( USE_CLEARCOAT ) 
		clearcoatSpecularIndirect *= ambientOcclusion;
	#endif
	#if defined( USE_SHEEN ) 
		sheenSpecularIndirect *= ambientOcclusion;
	#endif
	#if defined( USE_ENVMAP ) && defined( STANDARD )
		float dotNV = saturate( dot( geometryNormal, geometryViewDir ) );
		reflectedLight.indirectSpecular *= computeSpecularOcclusion( dotNV, ambientOcclusion, material.roughness );
	#endif
#endif`,iv=`#ifdef USE_AOMAP
	uniform sampler2D aoMap;
	uniform float aoMapIntensity;
#endif`,rv=`#ifdef USE_BATCHING
	attribute float batchId;
	uniform highp sampler2D batchingTexture;
	mat4 getBatchingMatrix( const in float i ) {
		int size = textureSize( batchingTexture, 0 ).x;
		int j = int( i ) * 4;
		int x = j % size;
		int y = j / size;
		vec4 v1 = texelFetch( batchingTexture, ivec2( x, y ), 0 );
		vec4 v2 = texelFetch( batchingTexture, ivec2( x + 1, y ), 0 );
		vec4 v3 = texelFetch( batchingTexture, ivec2( x + 2, y ), 0 );
		vec4 v4 = texelFetch( batchingTexture, ivec2( x + 3, y ), 0 );
		return mat4( v1, v2, v3, v4 );
	}
#endif`,sv=`#ifdef USE_BATCHING
	mat4 batchingMatrix = getBatchingMatrix( batchId );
#endif`,ov=`vec3 transformed = vec3( position );
#ifdef USE_ALPHAHASH
	vPosition = vec3( position );
#endif`,av=`vec3 objectNormal = vec3( normal );
#ifdef USE_TANGENT
	vec3 objectTangent = vec3( tangent.xyz );
#endif`,cv=`float G_BlinnPhong_Implicit( ) {
	return 0.25;
}
float D_BlinnPhong( const in float shininess, const in float dotNH ) {
	return RECIPROCAL_PI * ( shininess * 0.5 + 1.0 ) * pow( dotNH, shininess );
}
vec3 BRDF_BlinnPhong( const in vec3 lightDir, const in vec3 viewDir, const in vec3 normal, const in vec3 specularColor, const in float shininess ) {
	vec3 halfDir = normalize( lightDir + viewDir );
	float dotNH = saturate( dot( normal, halfDir ) );
	float dotVH = saturate( dot( viewDir, halfDir ) );
	vec3 F = F_Schlick( specularColor, 1.0, dotVH );
	float G = G_BlinnPhong_Implicit( );
	float D = D_BlinnPhong( shininess, dotNH );
	return F * ( G * D );
} // validated`,lv=`#ifdef USE_IRIDESCENCE
	const mat3 XYZ_TO_REC709 = mat3(
		 3.2404542, -0.9692660,  0.0556434,
		-1.5371385,  1.8760108, -0.2040259,
		-0.4985314,  0.0415560,  1.0572252
	);
	vec3 Fresnel0ToIor( vec3 fresnel0 ) {
		vec3 sqrtF0 = sqrt( fresnel0 );
		return ( vec3( 1.0 ) + sqrtF0 ) / ( vec3( 1.0 ) - sqrtF0 );
	}
	vec3 IorToFresnel0( vec3 transmittedIor, float incidentIor ) {
		return pow2( ( transmittedIor - vec3( incidentIor ) ) / ( transmittedIor + vec3( incidentIor ) ) );
	}
	float IorToFresnel0( float transmittedIor, float incidentIor ) {
		return pow2( ( transmittedIor - incidentIor ) / ( transmittedIor + incidentIor ));
	}
	vec3 evalSensitivity( float OPD, vec3 shift ) {
		float phase = 2.0 * PI * OPD * 1.0e-9;
		vec3 val = vec3( 5.4856e-13, 4.4201e-13, 5.2481e-13 );
		vec3 pos = vec3( 1.6810e+06, 1.7953e+06, 2.2084e+06 );
		vec3 var = vec3( 4.3278e+09, 9.3046e+09, 6.6121e+09 );
		vec3 xyz = val * sqrt( 2.0 * PI * var ) * cos( pos * phase + shift ) * exp( - pow2( phase ) * var );
		xyz.x += 9.7470e-14 * sqrt( 2.0 * PI * 4.5282e+09 ) * cos( 2.2399e+06 * phase + shift[ 0 ] ) * exp( - 4.5282e+09 * pow2( phase ) );
		xyz /= 1.0685e-7;
		vec3 rgb = XYZ_TO_REC709 * xyz;
		return rgb;
	}
	vec3 evalIridescence( float outsideIOR, float eta2, float cosTheta1, float thinFilmThickness, vec3 baseF0 ) {
		vec3 I;
		float iridescenceIOR = mix( outsideIOR, eta2, smoothstep( 0.0, 0.03, thinFilmThickness ) );
		float sinTheta2Sq = pow2( outsideIOR / iridescenceIOR ) * ( 1.0 - pow2( cosTheta1 ) );
		float cosTheta2Sq = 1.0 - sinTheta2Sq;
		if ( cosTheta2Sq < 0.0 ) {
			return vec3( 1.0 );
		}
		float cosTheta2 = sqrt( cosTheta2Sq );
		float R0 = IorToFresnel0( iridescenceIOR, outsideIOR );
		float R12 = F_Schlick( R0, 1.0, cosTheta1 );
		float T121 = 1.0 - R12;
		float phi12 = 0.0;
		if ( iridescenceIOR < outsideIOR ) phi12 = PI;
		float phi21 = PI - phi12;
		vec3 baseIOR = Fresnel0ToIor( clamp( baseF0, 0.0, 0.9999 ) );		vec3 R1 = IorToFresnel0( baseIOR, iridescenceIOR );
		vec3 R23 = F_Schlick( R1, 1.0, cosTheta2 );
		vec3 phi23 = vec3( 0.0 );
		if ( baseIOR[ 0 ] < iridescenceIOR ) phi23[ 0 ] = PI;
		if ( baseIOR[ 1 ] < iridescenceIOR ) phi23[ 1 ] = PI;
		if ( baseIOR[ 2 ] < iridescenceIOR ) phi23[ 2 ] = PI;
		float OPD = 2.0 * iridescenceIOR * thinFilmThickness * cosTheta2;
		vec3 phi = vec3( phi21 ) + phi23;
		vec3 R123 = clamp( R12 * R23, 1e-5, 0.9999 );
		vec3 r123 = sqrt( R123 );
		vec3 Rs = pow2( T121 ) * R23 / ( vec3( 1.0 ) - R123 );
		vec3 C0 = R12 + Rs;
		I = C0;
		vec3 Cm = Rs - T121;
		for ( int m = 1; m <= 2; ++ m ) {
			Cm *= r123;
			vec3 Sm = 2.0 * evalSensitivity( float( m ) * OPD, float( m ) * phi );
			I += Cm * Sm;
		}
		return max( I, vec3( 0.0 ) );
	}
#endif`,hv=`#ifdef USE_BUMPMAP
	uniform sampler2D bumpMap;
	uniform float bumpScale;
	vec2 dHdxy_fwd() {
		vec2 dSTdx = dFdx( vBumpMapUv );
		vec2 dSTdy = dFdy( vBumpMapUv );
		float Hll = bumpScale * texture2D( bumpMap, vBumpMapUv ).x;
		float dBx = bumpScale * texture2D( bumpMap, vBumpMapUv + dSTdx ).x - Hll;
		float dBy = bumpScale * texture2D( bumpMap, vBumpMapUv + dSTdy ).x - Hll;
		return vec2( dBx, dBy );
	}
	vec3 perturbNormalArb( vec3 surf_pos, vec3 surf_norm, vec2 dHdxy, float faceDirection ) {
		vec3 vSigmaX = normalize( dFdx( surf_pos.xyz ) );
		vec3 vSigmaY = normalize( dFdy( surf_pos.xyz ) );
		vec3 vN = surf_norm;
		vec3 R1 = cross( vSigmaY, vN );
		vec3 R2 = cross( vN, vSigmaX );
		float fDet = dot( vSigmaX, R1 ) * faceDirection;
		vec3 vGrad = sign( fDet ) * ( dHdxy.x * R1 + dHdxy.y * R2 );
		return normalize( abs( fDet ) * surf_norm - vGrad );
	}
#endif`,uv=`#if NUM_CLIPPING_PLANES > 0
	vec4 plane;
	#pragma unroll_loop_start
	for ( int i = 0; i < UNION_CLIPPING_PLANES; i ++ ) {
		plane = clippingPlanes[ i ];
		if ( dot( vClipPosition, plane.xyz ) > plane.w ) discard;
	}
	#pragma unroll_loop_end
	#if UNION_CLIPPING_PLANES < NUM_CLIPPING_PLANES
		bool clipped = true;
		#pragma unroll_loop_start
		for ( int i = UNION_CLIPPING_PLANES; i < NUM_CLIPPING_PLANES; i ++ ) {
			plane = clippingPlanes[ i ];
			clipped = ( dot( vClipPosition, plane.xyz ) > plane.w ) && clipped;
		}
		#pragma unroll_loop_end
		if ( clipped ) discard;
	#endif
#endif`,dv=`#if NUM_CLIPPING_PLANES > 0
	varying vec3 vClipPosition;
	uniform vec4 clippingPlanes[ NUM_CLIPPING_PLANES ];
#endif`,fv=`#if NUM_CLIPPING_PLANES > 0
	varying vec3 vClipPosition;
#endif`,pv=`#if NUM_CLIPPING_PLANES > 0
	vClipPosition = - mvPosition.xyz;
#endif`,mv=`#if defined( USE_COLOR_ALPHA )
	diffuseColor *= vColor;
#elif defined( USE_COLOR )
	diffuseColor.rgb *= vColor;
#endif`,gv=`#if defined( USE_COLOR_ALPHA )
	varying vec4 vColor;
#elif defined( USE_COLOR )
	varying vec3 vColor;
#endif`,_v=`#if defined( USE_COLOR_ALPHA )
	varying vec4 vColor;
#elif defined( USE_COLOR ) || defined( USE_INSTANCING_COLOR )
	varying vec3 vColor;
#endif`,vv=`#if defined( USE_COLOR_ALPHA )
	vColor = vec4( 1.0 );
#elif defined( USE_COLOR ) || defined( USE_INSTANCING_COLOR )
	vColor = vec3( 1.0 );
#endif
#ifdef USE_COLOR
	vColor *= color;
#endif
#ifdef USE_INSTANCING_COLOR
	vColor.xyz *= instanceColor.xyz;
#endif`,yv=`#define PI 3.141592653589793
#define PI2 6.283185307179586
#define PI_HALF 1.5707963267948966
#define RECIPROCAL_PI 0.3183098861837907
#define RECIPROCAL_PI2 0.15915494309189535
#define EPSILON 1e-6
#ifndef saturate
#define saturate( a ) clamp( a, 0.0, 1.0 )
#endif
#define whiteComplement( a ) ( 1.0 - saturate( a ) )
float pow2( const in float x ) { return x*x; }
vec3 pow2( const in vec3 x ) { return x*x; }
float pow3( const in float x ) { return x*x*x; }
float pow4( const in float x ) { float x2 = x*x; return x2*x2; }
float max3( const in vec3 v ) { return max( max( v.x, v.y ), v.z ); }
float average( const in vec3 v ) { return dot( v, vec3( 0.3333333 ) ); }
highp float rand( const in vec2 uv ) {
	const highp float a = 12.9898, b = 78.233, c = 43758.5453;
	highp float dt = dot( uv.xy, vec2( a,b ) ), sn = mod( dt, PI );
	return fract( sin( sn ) * c );
}
#ifdef HIGH_PRECISION
	float precisionSafeLength( vec3 v ) { return length( v ); }
#else
	float precisionSafeLength( vec3 v ) {
		float maxComponent = max3( abs( v ) );
		return length( v / maxComponent ) * maxComponent;
	}
#endif
struct IncidentLight {
	vec3 color;
	vec3 direction;
	bool visible;
};
struct ReflectedLight {
	vec3 directDiffuse;
	vec3 directSpecular;
	vec3 indirectDiffuse;
	vec3 indirectSpecular;
};
#ifdef USE_ALPHAHASH
	varying vec3 vPosition;
#endif
vec3 transformDirection( in vec3 dir, in mat4 matrix ) {
	return normalize( ( matrix * vec4( dir, 0.0 ) ).xyz );
}
vec3 inverseTransformDirection( in vec3 dir, in mat4 matrix ) {
	return normalize( ( vec4( dir, 0.0 ) * matrix ).xyz );
}
mat3 transposeMat3( const in mat3 m ) {
	mat3 tmp;
	tmp[ 0 ] = vec3( m[ 0 ].x, m[ 1 ].x, m[ 2 ].x );
	tmp[ 1 ] = vec3( m[ 0 ].y, m[ 1 ].y, m[ 2 ].y );
	tmp[ 2 ] = vec3( m[ 0 ].z, m[ 1 ].z, m[ 2 ].z );
	return tmp;
}
float luminance( const in vec3 rgb ) {
	const vec3 weights = vec3( 0.2126729, 0.7151522, 0.0721750 );
	return dot( weights, rgb );
}
bool isPerspectiveMatrix( mat4 m ) {
	return m[ 2 ][ 3 ] == - 1.0;
}
vec2 equirectUv( in vec3 dir ) {
	float u = atan( dir.z, dir.x ) * RECIPROCAL_PI2 + 0.5;
	float v = asin( clamp( dir.y, - 1.0, 1.0 ) ) * RECIPROCAL_PI + 0.5;
	return vec2( u, v );
}
vec3 BRDF_Lambert( const in vec3 diffuseColor ) {
	return RECIPROCAL_PI * diffuseColor;
}
vec3 F_Schlick( const in vec3 f0, const in float f90, const in float dotVH ) {
	float fresnel = exp2( ( - 5.55473 * dotVH - 6.98316 ) * dotVH );
	return f0 * ( 1.0 - fresnel ) + ( f90 * fresnel );
}
float F_Schlick( const in float f0, const in float f90, const in float dotVH ) {
	float fresnel = exp2( ( - 5.55473 * dotVH - 6.98316 ) * dotVH );
	return f0 * ( 1.0 - fresnel ) + ( f90 * fresnel );
} // validated`,Ev=`#ifdef ENVMAP_TYPE_CUBE_UV
	#define cubeUV_minMipLevel 4.0
	#define cubeUV_minTileSize 16.0
	float getFace( vec3 direction ) {
		vec3 absDirection = abs( direction );
		float face = - 1.0;
		if ( absDirection.x > absDirection.z ) {
			if ( absDirection.x > absDirection.y )
				face = direction.x > 0.0 ? 0.0 : 3.0;
			else
				face = direction.y > 0.0 ? 1.0 : 4.0;
		} else {
			if ( absDirection.z > absDirection.y )
				face = direction.z > 0.0 ? 2.0 : 5.0;
			else
				face = direction.y > 0.0 ? 1.0 : 4.0;
		}
		return face;
	}
	vec2 getUV( vec3 direction, float face ) {
		vec2 uv;
		if ( face == 0.0 ) {
			uv = vec2( direction.z, direction.y ) / abs( direction.x );
		} else if ( face == 1.0 ) {
			uv = vec2( - direction.x, - direction.z ) / abs( direction.y );
		} else if ( face == 2.0 ) {
			uv = vec2( - direction.x, direction.y ) / abs( direction.z );
		} else if ( face == 3.0 ) {
			uv = vec2( - direction.z, direction.y ) / abs( direction.x );
		} else if ( face == 4.0 ) {
			uv = vec2( - direction.x, direction.z ) / abs( direction.y );
		} else {
			uv = vec2( direction.x, direction.y ) / abs( direction.z );
		}
		return 0.5 * ( uv + 1.0 );
	}
	vec3 bilinearCubeUV( sampler2D envMap, vec3 direction, float mipInt ) {
		float face = getFace( direction );
		float filterInt = max( cubeUV_minMipLevel - mipInt, 0.0 );
		mipInt = max( mipInt, cubeUV_minMipLevel );
		float faceSize = exp2( mipInt );
		highp vec2 uv = getUV( direction, face ) * ( faceSize - 2.0 ) + 1.0;
		if ( face > 2.0 ) {
			uv.y += faceSize;
			face -= 3.0;
		}
		uv.x += face * faceSize;
		uv.x += filterInt * 3.0 * cubeUV_minTileSize;
		uv.y += 4.0 * ( exp2( CUBEUV_MAX_MIP ) - faceSize );
		uv.x *= CUBEUV_TEXEL_WIDTH;
		uv.y *= CUBEUV_TEXEL_HEIGHT;
		#ifdef texture2DGradEXT
			return texture2DGradEXT( envMap, uv, vec2( 0.0 ), vec2( 0.0 ) ).rgb;
		#else
			return texture2D( envMap, uv ).rgb;
		#endif
	}
	#define cubeUV_r0 1.0
	#define cubeUV_m0 - 2.0
	#define cubeUV_r1 0.8
	#define cubeUV_m1 - 1.0
	#define cubeUV_r4 0.4
	#define cubeUV_m4 2.0
	#define cubeUV_r5 0.305
	#define cubeUV_m5 3.0
	#define cubeUV_r6 0.21
	#define cubeUV_m6 4.0
	float roughnessToMip( float roughness ) {
		float mip = 0.0;
		if ( roughness >= cubeUV_r1 ) {
			mip = ( cubeUV_r0 - roughness ) * ( cubeUV_m1 - cubeUV_m0 ) / ( cubeUV_r0 - cubeUV_r1 ) + cubeUV_m0;
		} else if ( roughness >= cubeUV_r4 ) {
			mip = ( cubeUV_r1 - roughness ) * ( cubeUV_m4 - cubeUV_m1 ) / ( cubeUV_r1 - cubeUV_r4 ) + cubeUV_m1;
		} else if ( roughness >= cubeUV_r5 ) {
			mip = ( cubeUV_r4 - roughness ) * ( cubeUV_m5 - cubeUV_m4 ) / ( cubeUV_r4 - cubeUV_r5 ) + cubeUV_m4;
		} else if ( roughness >= cubeUV_r6 ) {
			mip = ( cubeUV_r5 - roughness ) * ( cubeUV_m6 - cubeUV_m5 ) / ( cubeUV_r5 - cubeUV_r6 ) + cubeUV_m5;
		} else {
			mip = - 2.0 * log2( 1.16 * roughness );		}
		return mip;
	}
	vec4 textureCubeUV( sampler2D envMap, vec3 sampleDir, float roughness ) {
		float mip = clamp( roughnessToMip( roughness ), cubeUV_m0, CUBEUV_MAX_MIP );
		float mipF = fract( mip );
		float mipInt = floor( mip );
		vec3 color0 = bilinearCubeUV( envMap, sampleDir, mipInt );
		if ( mipF == 0.0 ) {
			return vec4( color0, 1.0 );
		} else {
			vec3 color1 = bilinearCubeUV( envMap, sampleDir, mipInt + 1.0 );
			return vec4( mix( color0, color1, mipF ), 1.0 );
		}
	}
#endif`,xv=`vec3 transformedNormal = objectNormal;
#ifdef USE_TANGENT
	vec3 transformedTangent = objectTangent;
#endif
#ifdef USE_BATCHING
	mat3 bm = mat3( batchingMatrix );
	transformedNormal /= vec3( dot( bm[ 0 ], bm[ 0 ] ), dot( bm[ 1 ], bm[ 1 ] ), dot( bm[ 2 ], bm[ 2 ] ) );
	transformedNormal = bm * transformedNormal;
	#ifdef USE_TANGENT
		transformedTangent = bm * transformedTangent;
	#endif
#endif
#ifdef USE_INSTANCING
	mat3 im = mat3( instanceMatrix );
	transformedNormal /= vec3( dot( im[ 0 ], im[ 0 ] ), dot( im[ 1 ], im[ 1 ] ), dot( im[ 2 ], im[ 2 ] ) );
	transformedNormal = im * transformedNormal;
	#ifdef USE_TANGENT
		transformedTangent = im * transformedTangent;
	#endif
#endif
transformedNormal = normalMatrix * transformedNormal;
#ifdef FLIP_SIDED
	transformedNormal = - transformedNormal;
#endif
#ifdef USE_TANGENT
	transformedTangent = ( modelViewMatrix * vec4( transformedTangent, 0.0 ) ).xyz;
	#ifdef FLIP_SIDED
		transformedTangent = - transformedTangent;
	#endif
#endif`,Sv=`#ifdef USE_DISPLACEMENTMAP
	uniform sampler2D displacementMap;
	uniform float displacementScale;
	uniform float displacementBias;
#endif`,Tv=`#ifdef USE_DISPLACEMENTMAP
	transformed += normalize( objectNormal ) * ( texture2D( displacementMap, vDisplacementMapUv ).x * displacementScale + displacementBias );
#endif`,Mv=`#ifdef USE_EMISSIVEMAP
	vec4 emissiveColor = texture2D( emissiveMap, vEmissiveMapUv );
	totalEmissiveRadiance *= emissiveColor.rgb;
#endif`,wv=`#ifdef USE_EMISSIVEMAP
	uniform sampler2D emissiveMap;
#endif`,bv="gl_FragColor = linearToOutputTexel( gl_FragColor );",Av=`
const mat3 LINEAR_SRGB_TO_LINEAR_DISPLAY_P3 = mat3(
	vec3( 0.8224621, 0.177538, 0.0 ),
	vec3( 0.0331941, 0.9668058, 0.0 ),
	vec3( 0.0170827, 0.0723974, 0.9105199 )
);
const mat3 LINEAR_DISPLAY_P3_TO_LINEAR_SRGB = mat3(
	vec3( 1.2249401, - 0.2249404, 0.0 ),
	vec3( - 0.0420569, 1.0420571, 0.0 ),
	vec3( - 0.0196376, - 0.0786361, 1.0982735 )
);
vec4 LinearSRGBToLinearDisplayP3( in vec4 value ) {
	return vec4( value.rgb * LINEAR_SRGB_TO_LINEAR_DISPLAY_P3, value.a );
}
vec4 LinearDisplayP3ToLinearSRGB( in vec4 value ) {
	return vec4( value.rgb * LINEAR_DISPLAY_P3_TO_LINEAR_SRGB, value.a );
}
vec4 LinearTransferOETF( in vec4 value ) {
	return value;
}
vec4 sRGBTransferOETF( in vec4 value ) {
	return vec4( mix( pow( value.rgb, vec3( 0.41666 ) ) * 1.055 - vec3( 0.055 ), value.rgb * 12.92, vec3( lessThanEqual( value.rgb, vec3( 0.0031308 ) ) ) ), value.a );
}
vec4 LinearToLinear( in vec4 value ) {
	return value;
}
vec4 LinearTosRGB( in vec4 value ) {
	return sRGBTransferOETF( value );
}`,Rv=`#ifdef USE_ENVMAP
	#ifdef ENV_WORLDPOS
		vec3 cameraToFrag;
		if ( isOrthographic ) {
			cameraToFrag = normalize( vec3( - viewMatrix[ 0 ][ 2 ], - viewMatrix[ 1 ][ 2 ], - viewMatrix[ 2 ][ 2 ] ) );
		} else {
			cameraToFrag = normalize( vWorldPosition - cameraPosition );
		}
		vec3 worldNormal = inverseTransformDirection( normal, viewMatrix );
		#ifdef ENVMAP_MODE_REFLECTION
			vec3 reflectVec = reflect( cameraToFrag, worldNormal );
		#else
			vec3 reflectVec = refract( cameraToFrag, worldNormal, refractionRatio );
		#endif
	#else
		vec3 reflectVec = vReflect;
	#endif
	#ifdef ENVMAP_TYPE_CUBE
		vec4 envColor = textureCube( envMap, vec3( flipEnvMap * reflectVec.x, reflectVec.yz ) );
	#else
		vec4 envColor = vec4( 0.0 );
	#endif
	#ifdef ENVMAP_BLENDING_MULTIPLY
		outgoingLight = mix( outgoingLight, outgoingLight * envColor.xyz, specularStrength * reflectivity );
	#elif defined( ENVMAP_BLENDING_MIX )
		outgoingLight = mix( outgoingLight, envColor.xyz, specularStrength * reflectivity );
	#elif defined( ENVMAP_BLENDING_ADD )
		outgoingLight += envColor.xyz * specularStrength * reflectivity;
	#endif
#endif`,Cv=`#ifdef USE_ENVMAP
	uniform float envMapIntensity;
	uniform float flipEnvMap;
	#ifdef ENVMAP_TYPE_CUBE
		uniform samplerCube envMap;
	#else
		uniform sampler2D envMap;
	#endif
	
#endif`,Iv=`#ifdef USE_ENVMAP
	uniform float reflectivity;
	#if defined( USE_BUMPMAP ) || defined( USE_NORMALMAP ) || defined( PHONG ) || defined( LAMBERT )
		#define ENV_WORLDPOS
	#endif
	#ifdef ENV_WORLDPOS
		varying vec3 vWorldPosition;
		uniform float refractionRatio;
	#else
		varying vec3 vReflect;
	#endif
#endif`,Pv=`#ifdef USE_ENVMAP
	#if defined( USE_BUMPMAP ) || defined( USE_NORMALMAP ) || defined( PHONG ) || defined( LAMBERT )
		#define ENV_WORLDPOS
	#endif
	#ifdef ENV_WORLDPOS
		
		varying vec3 vWorldPosition;
	#else
		varying vec3 vReflect;
		uniform float refractionRatio;
	#endif
#endif`,Dv=`#ifdef USE_ENVMAP
	#ifdef ENV_WORLDPOS
		vWorldPosition = worldPosition.xyz;
	#else
		vec3 cameraToVertex;
		if ( isOrthographic ) {
			cameraToVertex = normalize( vec3( - viewMatrix[ 0 ][ 2 ], - viewMatrix[ 1 ][ 2 ], - viewMatrix[ 2 ][ 2 ] ) );
		} else {
			cameraToVertex = normalize( worldPosition.xyz - cameraPosition );
		}
		vec3 worldNormal = inverseTransformDirection( transformedNormal, viewMatrix );
		#ifdef ENVMAP_MODE_REFLECTION
			vReflect = reflect( cameraToVertex, worldNormal );
		#else
			vReflect = refract( cameraToVertex, worldNormal, refractionRatio );
		#endif
	#endif
#endif`,Lv=`#ifdef USE_FOG
	vFogDepth = - mvPosition.z;
#endif`,Nv=`#ifdef USE_FOG
	varying float vFogDepth;
#endif`,Uv=`#ifdef USE_FOG
	#ifdef FOG_EXP2
		float fogFactor = 1.0 - exp( - fogDensity * fogDensity * vFogDepth * vFogDepth );
	#else
		float fogFactor = smoothstep( fogNear, fogFar, vFogDepth );
	#endif
	gl_FragColor.rgb = mix( gl_FragColor.rgb, fogColor, fogFactor );
#endif`,Ov=`#ifdef USE_FOG
	uniform vec3 fogColor;
	varying float vFogDepth;
	#ifdef FOG_EXP2
		uniform float fogDensity;
	#else
		uniform float fogNear;
		uniform float fogFar;
	#endif
#endif`,Fv=`#ifdef USE_GRADIENTMAP
	uniform sampler2D gradientMap;
#endif
vec3 getGradientIrradiance( vec3 normal, vec3 lightDirection ) {
	float dotNL = dot( normal, lightDirection );
	vec2 coord = vec2( dotNL * 0.5 + 0.5, 0.0 );
	#ifdef USE_GRADIENTMAP
		return vec3( texture2D( gradientMap, coord ).r );
	#else
		vec2 fw = fwidth( coord ) * 0.5;
		return mix( vec3( 0.7 ), vec3( 1.0 ), smoothstep( 0.7 - fw.x, 0.7 + fw.x, coord.x ) );
	#endif
}`,Vv=`#ifdef USE_LIGHTMAP
	vec4 lightMapTexel = texture2D( lightMap, vLightMapUv );
	vec3 lightMapIrradiance = lightMapTexel.rgb * lightMapIntensity;
	reflectedLight.indirectDiffuse += lightMapIrradiance;
#endif`,Bv=`#ifdef USE_LIGHTMAP
	uniform sampler2D lightMap;
	uniform float lightMapIntensity;
#endif`,kv=`LambertMaterial material;
material.diffuseColor = diffuseColor.rgb;
material.specularStrength = specularStrength;`,Hv=`varying vec3 vViewPosition;
struct LambertMaterial {
	vec3 diffuseColor;
	float specularStrength;
};
void RE_Direct_Lambert( const in IncidentLight directLight, const in vec3 geometryPosition, const in vec3 geometryNormal, const in vec3 geometryViewDir, const in vec3 geometryClearcoatNormal, const in LambertMaterial material, inout ReflectedLight reflectedLight ) {
	float dotNL = saturate( dot( geometryNormal, directLight.direction ) );
	vec3 irradiance = dotNL * directLight.color;
	reflectedLight.directDiffuse += irradiance * BRDF_Lambert( material.diffuseColor );
}
void RE_IndirectDiffuse_Lambert( const in vec3 irradiance, const in vec3 geometryPosition, const in vec3 geometryNormal, const in vec3 geometryViewDir, const in vec3 geometryClearcoatNormal, const in LambertMaterial material, inout ReflectedLight reflectedLight ) {
	reflectedLight.indirectDiffuse += irradiance * BRDF_Lambert( material.diffuseColor );
}
#define RE_Direct				RE_Direct_Lambert
#define RE_IndirectDiffuse		RE_IndirectDiffuse_Lambert`,zv=`uniform bool receiveShadow;
uniform vec3 ambientLightColor;
#if defined( USE_LIGHT_PROBES )
	uniform vec3 lightProbe[ 9 ];
#endif
vec3 shGetIrradianceAt( in vec3 normal, in vec3 shCoefficients[ 9 ] ) {
	float x = normal.x, y = normal.y, z = normal.z;
	vec3 result = shCoefficients[ 0 ] * 0.886227;
	result += shCoefficients[ 1 ] * 2.0 * 0.511664 * y;
	result += shCoefficients[ 2 ] * 2.0 * 0.511664 * z;
	result += shCoefficients[ 3 ] * 2.0 * 0.511664 * x;
	result += shCoefficients[ 4 ] * 2.0 * 0.429043 * x * y;
	result += shCoefficients[ 5 ] * 2.0 * 0.429043 * y * z;
	result += shCoefficients[ 6 ] * ( 0.743125 * z * z - 0.247708 );
	result += shCoefficients[ 7 ] * 2.0 * 0.429043 * x * z;
	result += shCoefficients[ 8 ] * 0.429043 * ( x * x - y * y );
	return result;
}
vec3 getLightProbeIrradiance( const in vec3 lightProbe[ 9 ], const in vec3 normal ) {
	vec3 worldNormal = inverseTransformDirection( normal, viewMatrix );
	vec3 irradiance = shGetIrradianceAt( worldNormal, lightProbe );
	return irradiance;
}
vec3 getAmbientLightIrradiance( const in vec3 ambientLightColor ) {
	vec3 irradiance = ambientLightColor;
	return irradiance;
}
float getDistanceAttenuation( const in float lightDistance, const in float cutoffDistance, const in float decayExponent ) {
	#if defined ( LEGACY_LIGHTS )
		if ( cutoffDistance > 0.0 && decayExponent > 0.0 ) {
			return pow( saturate( - lightDistance / cutoffDistance + 1.0 ), decayExponent );
		}
		return 1.0;
	#else
		float distanceFalloff = 1.0 / max( pow( lightDistance, decayExponent ), 0.01 );
		if ( cutoffDistance > 0.0 ) {
			distanceFalloff *= pow2( saturate( 1.0 - pow4( lightDistance / cutoffDistance ) ) );
		}
		return distanceFalloff;
	#endif
}
float getSpotAttenuation( const in float coneCosine, const in float penumbraCosine, const in float angleCosine ) {
	return smoothstep( coneCosine, penumbraCosine, angleCosine );
}
#if NUM_DIR_LIGHTS > 0
	struct DirectionalLight {
		vec3 direction;
		vec3 color;
	};
	uniform DirectionalLight directionalLights[ NUM_DIR_LIGHTS ];
	void getDirectionalLightInfo( const in DirectionalLight directionalLight, out IncidentLight light ) {
		light.color = directionalLight.color;
		light.direction = directionalLight.direction;
		light.visible = true;
	}
#endif
#if NUM_POINT_LIGHTS > 0
	struct PointLight {
		vec3 position;
		vec3 color;
		float distance;
		float decay;
	};
	uniform PointLight pointLights[ NUM_POINT_LIGHTS ];
	void getPointLightInfo( const in PointLight pointLight, const in vec3 geometryPosition, out IncidentLight light ) {
		vec3 lVector = pointLight.position - geometryPosition;
		light.direction = normalize( lVector );
		float lightDistance = length( lVector );
		light.color = pointLight.color;
		light.color *= getDistanceAttenuation( lightDistance, pointLight.distance, pointLight.decay );
		light.visible = ( light.color != vec3( 0.0 ) );
	}
#endif
#if NUM_SPOT_LIGHTS > 0
	struct SpotLight {
		vec3 position;
		vec3 direction;
		vec3 color;
		float distance;
		float decay;
		float coneCos;
		float penumbraCos;
	};
	uniform SpotLight spotLights[ NUM_SPOT_LIGHTS ];
	void getSpotLightInfo( const in SpotLight spotLight, const in vec3 geometryPosition, out IncidentLight light ) {
		vec3 lVector = spotLight.position - geometryPosition;
		light.direction = normalize( lVector );
		float angleCos = dot( light.direction, spotLight.direction );
		float spotAttenuation = getSpotAttenuation( spotLight.coneCos, spotLight.penumbraCos, angleCos );
		if ( spotAttenuation > 0.0 ) {
			float lightDistance = length( lVector );
			light.color = spotLight.color * spotAttenuation;
			light.color *= getDistanceAttenuation( lightDistance, spotLight.distance, spotLight.decay );
			light.visible = ( light.color != vec3( 0.0 ) );
		} else {
			light.color = vec3( 0.0 );
			light.visible = false;
		}
	}
#endif
#if NUM_RECT_AREA_LIGHTS > 0
	struct RectAreaLight {
		vec3 color;
		vec3 position;
		vec3 halfWidth;
		vec3 halfHeight;
	};
	uniform sampler2D ltc_1;	uniform sampler2D ltc_2;
	uniform RectAreaLight rectAreaLights[ NUM_RECT_AREA_LIGHTS ];
#endif
#if NUM_HEMI_LIGHTS > 0
	struct HemisphereLight {
		vec3 direction;
		vec3 skyColor;
		vec3 groundColor;
	};
	uniform HemisphereLight hemisphereLights[ NUM_HEMI_LIGHTS ];
	vec3 getHemisphereLightIrradiance( const in HemisphereLight hemiLight, const in vec3 normal ) {
		float dotNL = dot( normal, hemiLight.direction );
		float hemiDiffuseWeight = 0.5 * dotNL + 0.5;
		vec3 irradiance = mix( hemiLight.groundColor, hemiLight.skyColor, hemiDiffuseWeight );
		return irradiance;
	}
#endif`,Gv=`#ifdef USE_ENVMAP
	vec3 getIBLIrradiance( const in vec3 normal ) {
		#ifdef ENVMAP_TYPE_CUBE_UV
			vec3 worldNormal = inverseTransformDirection( normal, viewMatrix );
			vec4 envMapColor = textureCubeUV( envMap, worldNormal, 1.0 );
			return PI * envMapColor.rgb * envMapIntensity;
		#else
			return vec3( 0.0 );
		#endif
	}
	vec3 getIBLRadiance( const in vec3 viewDir, const in vec3 normal, const in float roughness ) {
		#ifdef ENVMAP_TYPE_CUBE_UV
			vec3 reflectVec = reflect( - viewDir, normal );
			reflectVec = normalize( mix( reflectVec, normal, roughness * roughness) );
			reflectVec = inverseTransformDirection( reflectVec, viewMatrix );
			vec4 envMapColor = textureCubeUV( envMap, reflectVec, roughness );
			return envMapColor.rgb * envMapIntensity;
		#else
			return vec3( 0.0 );
		#endif
	}
	#ifdef USE_ANISOTROPY
		vec3 getIBLAnisotropyRadiance( const in vec3 viewDir, const in vec3 normal, const in float roughness, const in vec3 bitangent, const in float anisotropy ) {
			#ifdef ENVMAP_TYPE_CUBE_UV
				vec3 bentNormal = cross( bitangent, viewDir );
				bentNormal = normalize( cross( bentNormal, bitangent ) );
				bentNormal = normalize( mix( bentNormal, normal, pow2( pow2( 1.0 - anisotropy * ( 1.0 - roughness ) ) ) ) );
				return getIBLRadiance( viewDir, bentNormal, roughness );
			#else
				return vec3( 0.0 );
			#endif
		}
	#endif
#endif`,Wv=`ToonMaterial material;
material.diffuseColor = diffuseColor.rgb;`,qv=`varying vec3 vViewPosition;
struct ToonMaterial {
	vec3 diffuseColor;
};
void RE_Direct_Toon( const in IncidentLight directLight, const in vec3 geometryPosition, const in vec3 geometryNormal, const in vec3 geometryViewDir, const in vec3 geometryClearcoatNormal, const in ToonMaterial material, inout ReflectedLight reflectedLight ) {
	vec3 irradiance = getGradientIrradiance( geometryNormal, directLight.direction ) * directLight.color;
	reflectedLight.directDiffuse += irradiance * BRDF_Lambert( material.diffuseColor );
}
void RE_IndirectDiffuse_Toon( const in vec3 irradiance, const in vec3 geometryPosition, const in vec3 geometryNormal, const in vec3 geometryViewDir, const in vec3 geometryClearcoatNormal, const in ToonMaterial material, inout ReflectedLight reflectedLight ) {
	reflectedLight.indirectDiffuse += irradiance * BRDF_Lambert( material.diffuseColor );
}
#define RE_Direct				RE_Direct_Toon
#define RE_IndirectDiffuse		RE_IndirectDiffuse_Toon`,Xv=`BlinnPhongMaterial material;
material.diffuseColor = diffuseColor.rgb;
material.specularColor = specular;
material.specularShininess = shininess;
material.specularStrength = specularStrength;`,jv=`varying vec3 vViewPosition;
struct BlinnPhongMaterial {
	vec3 diffuseColor;
	vec3 specularColor;
	float specularShininess;
	float specularStrength;
};
void RE_Direct_BlinnPhong( const in IncidentLight directLight, const in vec3 geometryPosition, const in vec3 geometryNormal, const in vec3 geometryViewDir, const in vec3 geometryClearcoatNormal, const in BlinnPhongMaterial material, inout ReflectedLight reflectedLight ) {
	float dotNL = saturate( dot( geometryNormal, directLight.direction ) );
	vec3 irradiance = dotNL * directLight.color;
	reflectedLight.directDiffuse += irradiance * BRDF_Lambert( material.diffuseColor );
	reflectedLight.directSpecular += irradiance * BRDF_BlinnPhong( directLight.direction, geometryViewDir, geometryNormal, material.specularColor, material.specularShininess ) * material.specularStrength;
}
void RE_IndirectDiffuse_BlinnPhong( const in vec3 irradiance, const in vec3 geometryPosition, const in vec3 geometryNormal, const in vec3 geometryViewDir, const in vec3 geometryClearcoatNormal, const in BlinnPhongMaterial material, inout ReflectedLight reflectedLight ) {
	reflectedLight.indirectDiffuse += irradiance * BRDF_Lambert( material.diffuseColor );
}
#define RE_Direct				RE_Direct_BlinnPhong
#define RE_IndirectDiffuse		RE_IndirectDiffuse_BlinnPhong`,Yv=`PhysicalMaterial material;
material.diffuseColor = diffuseColor.rgb * ( 1.0 - metalnessFactor );
vec3 dxy = max( abs( dFdx( nonPerturbedNormal ) ), abs( dFdy( nonPerturbedNormal ) ) );
float geometryRoughness = max( max( dxy.x, dxy.y ), dxy.z );
material.roughness = max( roughnessFactor, 0.0525 );material.roughness += geometryRoughness;
material.roughness = min( material.roughness, 1.0 );
#ifdef IOR
	material.ior = ior;
	#ifdef USE_SPECULAR
		float specularIntensityFactor = specularIntensity;
		vec3 specularColorFactor = specularColor;
		#ifdef USE_SPECULAR_COLORMAP
			specularColorFactor *= texture2D( specularColorMap, vSpecularColorMapUv ).rgb;
		#endif
		#ifdef USE_SPECULAR_INTENSITYMAP
			specularIntensityFactor *= texture2D( specularIntensityMap, vSpecularIntensityMapUv ).a;
		#endif
		material.specularF90 = mix( specularIntensityFactor, 1.0, metalnessFactor );
	#else
		float specularIntensityFactor = 1.0;
		vec3 specularColorFactor = vec3( 1.0 );
		material.specularF90 = 1.0;
	#endif
	material.specularColor = mix( min( pow2( ( material.ior - 1.0 ) / ( material.ior + 1.0 ) ) * specularColorFactor, vec3( 1.0 ) ) * specularIntensityFactor, diffuseColor.rgb, metalnessFactor );
#else
	material.specularColor = mix( vec3( 0.04 ), diffuseColor.rgb, metalnessFactor );
	material.specularF90 = 1.0;
#endif
#ifdef USE_CLEARCOAT
	material.clearcoat = clearcoat;
	material.clearcoatRoughness = clearcoatRoughness;
	material.clearcoatF0 = vec3( 0.04 );
	material.clearcoatF90 = 1.0;
	#ifdef USE_CLEARCOATMAP
		material.clearcoat *= texture2D( clearcoatMap, vClearcoatMapUv ).x;
	#endif
	#ifdef USE_CLEARCOAT_ROUGHNESSMAP
		material.clearcoatRoughness *= texture2D( clearcoatRoughnessMap, vClearcoatRoughnessMapUv ).y;
	#endif
	material.clearcoat = saturate( material.clearcoat );	material.clearcoatRoughness = max( material.clearcoatRoughness, 0.0525 );
	material.clearcoatRoughness += geometryRoughness;
	material.clearcoatRoughness = min( material.clearcoatRoughness, 1.0 );
#endif
#ifdef USE_IRIDESCENCE
	material.iridescence = iridescence;
	material.iridescenceIOR = iridescenceIOR;
	#ifdef USE_IRIDESCENCEMAP
		material.iridescence *= texture2D( iridescenceMap, vIridescenceMapUv ).r;
	#endif
	#ifdef USE_IRIDESCENCE_THICKNESSMAP
		material.iridescenceThickness = (iridescenceThicknessMaximum - iridescenceThicknessMinimum) * texture2D( iridescenceThicknessMap, vIridescenceThicknessMapUv ).g + iridescenceThicknessMinimum;
	#else
		material.iridescenceThickness = iridescenceThicknessMaximum;
	#endif
#endif
#ifdef USE_SHEEN
	material.sheenColor = sheenColor;
	#ifdef USE_SHEEN_COLORMAP
		material.sheenColor *= texture2D( sheenColorMap, vSheenColorMapUv ).rgb;
	#endif
	material.sheenRoughness = clamp( sheenRoughness, 0.07, 1.0 );
	#ifdef USE_SHEEN_ROUGHNESSMAP
		material.sheenRoughness *= texture2D( sheenRoughnessMap, vSheenRoughnessMapUv ).a;
	#endif
#endif
#ifdef USE_ANISOTROPY
	#ifdef USE_ANISOTROPYMAP
		mat2 anisotropyMat = mat2( anisotropyVector.x, anisotropyVector.y, - anisotropyVector.y, anisotropyVector.x );
		vec3 anisotropyPolar = texture2D( anisotropyMap, vAnisotropyMapUv ).rgb;
		vec2 anisotropyV = anisotropyMat * normalize( 2.0 * anisotropyPolar.rg - vec2( 1.0 ) ) * anisotropyPolar.b;
	#else
		vec2 anisotropyV = anisotropyVector;
	#endif
	material.anisotropy = length( anisotropyV );
	if( material.anisotropy == 0.0 ) {
		anisotropyV = vec2( 1.0, 0.0 );
	} else {
		anisotropyV /= material.anisotropy;
		material.anisotropy = saturate( material.anisotropy );
	}
	material.alphaT = mix( pow2( material.roughness ), 1.0, pow2( material.anisotropy ) );
	material.anisotropyT = tbn[ 0 ] * anisotropyV.x + tbn[ 1 ] * anisotropyV.y;
	material.anisotropyB = tbn[ 1 ] * anisotropyV.x - tbn[ 0 ] * anisotropyV.y;
#endif`,$v=`struct PhysicalMaterial {
	vec3 diffuseColor;
	float roughness;
	vec3 specularColor;
	float specularF90;
	#ifdef USE_CLEARCOAT
		float clearcoat;
		float clearcoatRoughness;
		vec3 clearcoatF0;
		float clearcoatF90;
	#endif
	#ifdef USE_IRIDESCENCE
		float iridescence;
		float iridescenceIOR;
		float iridescenceThickness;
		vec3 iridescenceFresnel;
		vec3 iridescenceF0;
	#endif
	#ifdef USE_SHEEN
		vec3 sheenColor;
		float sheenRoughness;
	#endif
	#ifdef IOR
		float ior;
	#endif
	#ifdef USE_TRANSMISSION
		float transmission;
		float transmissionAlpha;
		float thickness;
		float attenuationDistance;
		vec3 attenuationColor;
	#endif
	#ifdef USE_ANISOTROPY
		float anisotropy;
		float alphaT;
		vec3 anisotropyT;
		vec3 anisotropyB;
	#endif
};
vec3 clearcoatSpecularDirect = vec3( 0.0 );
vec3 clearcoatSpecularIndirect = vec3( 0.0 );
vec3 sheenSpecularDirect = vec3( 0.0 );
vec3 sheenSpecularIndirect = vec3(0.0 );
vec3 Schlick_to_F0( const in vec3 f, const in float f90, const in float dotVH ) {
    float x = clamp( 1.0 - dotVH, 0.0, 1.0 );
    float x2 = x * x;
    float x5 = clamp( x * x2 * x2, 0.0, 0.9999 );
    return ( f - vec3( f90 ) * x5 ) / ( 1.0 - x5 );
}
float V_GGX_SmithCorrelated( const in float alpha, const in float dotNL, const in float dotNV ) {
	float a2 = pow2( alpha );
	float gv = dotNL * sqrt( a2 + ( 1.0 - a2 ) * pow2( dotNV ) );
	float gl = dotNV * sqrt( a2 + ( 1.0 - a2 ) * pow2( dotNL ) );
	return 0.5 / max( gv + gl, EPSILON );
}
float D_GGX( const in float alpha, const in float dotNH ) {
	float a2 = pow2( alpha );
	float denom = pow2( dotNH ) * ( a2 - 1.0 ) + 1.0;
	return RECIPROCAL_PI * a2 / pow2( denom );
}
#ifdef USE_ANISOTROPY
	float V_GGX_SmithCorrelated_Anisotropic( const in float alphaT, const in float alphaB, const in float dotTV, const in float dotBV, const in float dotTL, const in float dotBL, const in float dotNV, const in float dotNL ) {
		float gv = dotNL * length( vec3( alphaT * dotTV, alphaB * dotBV, dotNV ) );
		float gl = dotNV * length( vec3( alphaT * dotTL, alphaB * dotBL, dotNL ) );
		float v = 0.5 / ( gv + gl );
		return saturate(v);
	}
	float D_GGX_Anisotropic( const in float alphaT, const in float alphaB, const in float dotNH, const in float dotTH, const in float dotBH ) {
		float a2 = alphaT * alphaB;
		highp vec3 v = vec3( alphaB * dotTH, alphaT * dotBH, a2 * dotNH );
		highp float v2 = dot( v, v );
		float w2 = a2 / v2;
		return RECIPROCAL_PI * a2 * pow2 ( w2 );
	}
#endif
#ifdef USE_CLEARCOAT
	vec3 BRDF_GGX_Clearcoat( const in vec3 lightDir, const in vec3 viewDir, const in vec3 normal, const in PhysicalMaterial material) {
		vec3 f0 = material.clearcoatF0;
		float f90 = material.clearcoatF90;
		float roughness = material.clearcoatRoughness;
		float alpha = pow2( roughness );
		vec3 halfDir = normalize( lightDir + viewDir );
		float dotNL = saturate( dot( normal, lightDir ) );
		float dotNV = saturate( dot( normal, viewDir ) );
		float dotNH = saturate( dot( normal, halfDir ) );
		float dotVH = saturate( dot( viewDir, halfDir ) );
		vec3 F = F_Schlick( f0, f90, dotVH );
		float V = V_GGX_SmithCorrelated( alpha, dotNL, dotNV );
		float D = D_GGX( alpha, dotNH );
		return F * ( V * D );
	}
#endif
vec3 BRDF_GGX( const in vec3 lightDir, const in vec3 viewDir, const in vec3 normal, const in PhysicalMaterial material ) {
	vec3 f0 = material.specularColor;
	float f90 = material.specularF90;
	float roughness = material.roughness;
	float alpha = pow2( roughness );
	vec3 halfDir = normalize( lightDir + viewDir );
	float dotNL = saturate( dot( normal, lightDir ) );
	float dotNV = saturate( dot( normal, viewDir ) );
	float dotNH = saturate( dot( normal, halfDir ) );
	float dotVH = saturate( dot( viewDir, halfDir ) );
	vec3 F = F_Schlick( f0, f90, dotVH );
	#ifdef USE_IRIDESCENCE
		F = mix( F, material.iridescenceFresnel, material.iridescence );
	#endif
	#ifdef USE_ANISOTROPY
		float dotTL = dot( material.anisotropyT, lightDir );
		float dotTV = dot( material.anisotropyT, viewDir );
		float dotTH = dot( material.anisotropyT, halfDir );
		float dotBL = dot( material.anisotropyB, lightDir );
		float dotBV = dot( material.anisotropyB, viewDir );
		float dotBH = dot( material.anisotropyB, halfDir );
		float V = V_GGX_SmithCorrelated_Anisotropic( material.alphaT, alpha, dotTV, dotBV, dotTL, dotBL, dotNV, dotNL );
		float D = D_GGX_Anisotropic( material.alphaT, alpha, dotNH, dotTH, dotBH );
	#else
		float V = V_GGX_SmithCorrelated( alpha, dotNL, dotNV );
		float D = D_GGX( alpha, dotNH );
	#endif
	return F * ( V * D );
}
vec2 LTC_Uv( const in vec3 N, const in vec3 V, const in float roughness ) {
	const float LUT_SIZE = 64.0;
	const float LUT_SCALE = ( LUT_SIZE - 1.0 ) / LUT_SIZE;
	const float LUT_BIAS = 0.5 / LUT_SIZE;
	float dotNV = saturate( dot( N, V ) );
	vec2 uv = vec2( roughness, sqrt( 1.0 - dotNV ) );
	uv = uv * LUT_SCALE + LUT_BIAS;
	return uv;
}
float LTC_ClippedSphereFormFactor( const in vec3 f ) {
	float l = length( f );
	return max( ( l * l + f.z ) / ( l + 1.0 ), 0.0 );
}
vec3 LTC_EdgeVectorFormFactor( const in vec3 v1, const in vec3 v2 ) {
	float x = dot( v1, v2 );
	float y = abs( x );
	float a = 0.8543985 + ( 0.4965155 + 0.0145206 * y ) * y;
	float b = 3.4175940 + ( 4.1616724 + y ) * y;
	float v = a / b;
	float theta_sintheta = ( x > 0.0 ) ? v : 0.5 * inversesqrt( max( 1.0 - x * x, 1e-7 ) ) - v;
	return cross( v1, v2 ) * theta_sintheta;
}
vec3 LTC_Evaluate( const in vec3 N, const in vec3 V, const in vec3 P, const in mat3 mInv, const in vec3 rectCoords[ 4 ] ) {
	vec3 v1 = rectCoords[ 1 ] - rectCoords[ 0 ];
	vec3 v2 = rectCoords[ 3 ] - rectCoords[ 0 ];
	vec3 lightNormal = cross( v1, v2 );
	if( dot( lightNormal, P - rectCoords[ 0 ] ) < 0.0 ) return vec3( 0.0 );
	vec3 T1, T2;
	T1 = normalize( V - N * dot( V, N ) );
	T2 = - cross( N, T1 );
	mat3 mat = mInv * transposeMat3( mat3( T1, T2, N ) );
	vec3 coords[ 4 ];
	coords[ 0 ] = mat * ( rectCoords[ 0 ] - P );
	coords[ 1 ] = mat * ( rectCoords[ 1 ] - P );
	coords[ 2 ] = mat * ( rectCoords[ 2 ] - P );
	coords[ 3 ] = mat * ( rectCoords[ 3 ] - P );
	coords[ 0 ] = normalize( coords[ 0 ] );
	coords[ 1 ] = normalize( coords[ 1 ] );
	coords[ 2 ] = normalize( coords[ 2 ] );
	coords[ 3 ] = normalize( coords[ 3 ] );
	vec3 vectorFormFactor = vec3( 0.0 );
	vectorFormFactor += LTC_EdgeVectorFormFactor( coords[ 0 ], coords[ 1 ] );
	vectorFormFactor += LTC_EdgeVectorFormFactor( coords[ 1 ], coords[ 2 ] );
	vectorFormFactor += LTC_EdgeVectorFormFactor( coords[ 2 ], coords[ 3 ] );
	vectorFormFactor += LTC_EdgeVectorFormFactor( coords[ 3 ], coords[ 0 ] );
	float result = LTC_ClippedSphereFormFactor( vectorFormFactor );
	return vec3( result );
}
#if defined( USE_SHEEN )
float D_Charlie( float roughness, float dotNH ) {
	float alpha = pow2( roughness );
	float invAlpha = 1.0 / alpha;
	float cos2h = dotNH * dotNH;
	float sin2h = max( 1.0 - cos2h, 0.0078125 );
	return ( 2.0 + invAlpha ) * pow( sin2h, invAlpha * 0.5 ) / ( 2.0 * PI );
}
float V_Neubelt( float dotNV, float dotNL ) {
	return saturate( 1.0 / ( 4.0 * ( dotNL + dotNV - dotNL * dotNV ) ) );
}
vec3 BRDF_Sheen( const in vec3 lightDir, const in vec3 viewDir, const in vec3 normal, vec3 sheenColor, const in float sheenRoughness ) {
	vec3 halfDir = normalize( lightDir + viewDir );
	float dotNL = saturate( dot( normal, lightDir ) );
	float dotNV = saturate( dot( normal, viewDir ) );
	float dotNH = saturate( dot( normal, halfDir ) );
	float D = D_Charlie( sheenRoughness, dotNH );
	float V = V_Neubelt( dotNV, dotNL );
	return sheenColor * ( D * V );
}
#endif
float IBLSheenBRDF( const in vec3 normal, const in vec3 viewDir, const in float roughness ) {
	float dotNV = saturate( dot( normal, viewDir ) );
	float r2 = roughness * roughness;
	float a = roughness < 0.25 ? -339.2 * r2 + 161.4 * roughness - 25.9 : -8.48 * r2 + 14.3 * roughness - 9.95;
	float b = roughness < 0.25 ? 44.0 * r2 - 23.7 * roughness + 3.26 : 1.97 * r2 - 3.27 * roughness + 0.72;
	float DG = exp( a * dotNV + b ) + ( roughness < 0.25 ? 0.0 : 0.1 * ( roughness - 0.25 ) );
	return saturate( DG * RECIPROCAL_PI );
}
vec2 DFGApprox( const in vec3 normal, const in vec3 viewDir, const in float roughness ) {
	float dotNV = saturate( dot( normal, viewDir ) );
	const vec4 c0 = vec4( - 1, - 0.0275, - 0.572, 0.022 );
	const vec4 c1 = vec4( 1, 0.0425, 1.04, - 0.04 );
	vec4 r = roughness * c0 + c1;
	float a004 = min( r.x * r.x, exp2( - 9.28 * dotNV ) ) * r.x + r.y;
	vec2 fab = vec2( - 1.04, 1.04 ) * a004 + r.zw;
	return fab;
}
vec3 EnvironmentBRDF( const in vec3 normal, const in vec3 viewDir, const in vec3 specularColor, const in float specularF90, const in float roughness ) {
	vec2 fab = DFGApprox( normal, viewDir, roughness );
	return specularColor * fab.x + specularF90 * fab.y;
}
#ifdef USE_IRIDESCENCE
void computeMultiscatteringIridescence( const in vec3 normal, const in vec3 viewDir, const in vec3 specularColor, const in float specularF90, const in float iridescence, const in vec3 iridescenceF0, const in float roughness, inout vec3 singleScatter, inout vec3 multiScatter ) {
#else
void computeMultiscattering( const in vec3 normal, const in vec3 viewDir, const in vec3 specularColor, const in float specularF90, const in float roughness, inout vec3 singleScatter, inout vec3 multiScatter ) {
#endif
	vec2 fab = DFGApprox( normal, viewDir, roughness );
	#ifdef USE_IRIDESCENCE
		vec3 Fr = mix( specularColor, iridescenceF0, iridescence );
	#else
		vec3 Fr = specularColor;
	#endif
	vec3 FssEss = Fr * fab.x + specularF90 * fab.y;
	float Ess = fab.x + fab.y;
	float Ems = 1.0 - Ess;
	vec3 Favg = Fr + ( 1.0 - Fr ) * 0.047619;	vec3 Fms = FssEss * Favg / ( 1.0 - Ems * Favg );
	singleScatter += FssEss;
	multiScatter += Fms * Ems;
}
#if NUM_RECT_AREA_LIGHTS > 0
	void RE_Direct_RectArea_Physical( const in RectAreaLight rectAreaLight, const in vec3 geometryPosition, const in vec3 geometryNormal, const in vec3 geometryViewDir, const in vec3 geometryClearcoatNormal, const in PhysicalMaterial material, inout ReflectedLight reflectedLight ) {
		vec3 normal = geometryNormal;
		vec3 viewDir = geometryViewDir;
		vec3 position = geometryPosition;
		vec3 lightPos = rectAreaLight.position;
		vec3 halfWidth = rectAreaLight.halfWidth;
		vec3 halfHeight = rectAreaLight.halfHeight;
		vec3 lightColor = rectAreaLight.color;
		float roughness = material.roughness;
		vec3 rectCoords[ 4 ];
		rectCoords[ 0 ] = lightPos + halfWidth - halfHeight;		rectCoords[ 1 ] = lightPos - halfWidth - halfHeight;
		rectCoords[ 2 ] = lightPos - halfWidth + halfHeight;
		rectCoords[ 3 ] = lightPos + halfWidth + halfHeight;
		vec2 uv = LTC_Uv( normal, viewDir, roughness );
		vec4 t1 = texture2D( ltc_1, uv );
		vec4 t2 = texture2D( ltc_2, uv );
		mat3 mInv = mat3(
			vec3( t1.x, 0, t1.y ),
			vec3(    0, 1,    0 ),
			vec3( t1.z, 0, t1.w )
		);
		vec3 fresnel = ( material.specularColor * t2.x + ( vec3( 1.0 ) - material.specularColor ) * t2.y );
		reflectedLight.directSpecular += lightColor * fresnel * LTC_Evaluate( normal, viewDir, position, mInv, rectCoords );
		reflectedLight.directDiffuse += lightColor * material.diffuseColor * LTC_Evaluate( normal, viewDir, position, mat3( 1.0 ), rectCoords );
	}
#endif
void RE_Direct_Physical( const in IncidentLight directLight, const in vec3 geometryPosition, const in vec3 geometryNormal, const in vec3 geometryViewDir, const in vec3 geometryClearcoatNormal, const in PhysicalMaterial material, inout ReflectedLight reflectedLight ) {
	float dotNL = saturate( dot( geometryNormal, directLight.direction ) );
	vec3 irradiance = dotNL * directLight.color;
	#ifdef USE_CLEARCOAT
		float dotNLcc = saturate( dot( geometryClearcoatNormal, directLight.direction ) );
		vec3 ccIrradiance = dotNLcc * directLight.color;
		clearcoatSpecularDirect += ccIrradiance * BRDF_GGX_Clearcoat( directLight.direction, geometryViewDir, geometryClearcoatNormal, material );
	#endif
	#ifdef USE_SHEEN
		sheenSpecularDirect += irradiance * BRDF_Sheen( directLight.direction, geometryViewDir, geometryNormal, material.sheenColor, material.sheenRoughness );
	#endif
	reflectedLight.directSpecular += irradiance * BRDF_GGX( directLight.direction, geometryViewDir, geometryNormal, material );
	reflectedLight.directDiffuse += irradiance * BRDF_Lambert( material.diffuseColor );
}
void RE_IndirectDiffuse_Physical( const in vec3 irradiance, const in vec3 geometryPosition, const in vec3 geometryNormal, const in vec3 geometryViewDir, const in vec3 geometryClearcoatNormal, const in PhysicalMaterial material, inout ReflectedLight reflectedLight ) {
	reflectedLight.indirectDiffuse += irradiance * BRDF_Lambert( material.diffuseColor );
}
void RE_IndirectSpecular_Physical( const in vec3 radiance, const in vec3 irradiance, const in vec3 clearcoatRadiance, const in vec3 geometryPosition, const in vec3 geometryNormal, const in vec3 geometryViewDir, const in vec3 geometryClearcoatNormal, const in PhysicalMaterial material, inout ReflectedLight reflectedLight) {
	#ifdef USE_CLEARCOAT
		clearcoatSpecularIndirect += clearcoatRadiance * EnvironmentBRDF( geometryClearcoatNormal, geometryViewDir, material.clearcoatF0, material.clearcoatF90, material.clearcoatRoughness );
	#endif
	#ifdef USE_SHEEN
		sheenSpecularIndirect += irradiance * material.sheenColor * IBLSheenBRDF( geometryNormal, geometryViewDir, material.sheenRoughness );
	#endif
	vec3 singleScattering = vec3( 0.0 );
	vec3 multiScattering = vec3( 0.0 );
	vec3 cosineWeightedIrradiance = irradiance * RECIPROCAL_PI;
	#ifdef USE_IRIDESCENCE
		computeMultiscatteringIridescence( geometryNormal, geometryViewDir, material.specularColor, material.specularF90, material.iridescence, material.iridescenceFresnel, material.roughness, singleScattering, multiScattering );
	#else
		computeMultiscattering( geometryNormal, geometryViewDir, material.specularColor, material.specularF90, material.roughness, singleScattering, multiScattering );
	#endif
	vec3 totalScattering = singleScattering + multiScattering;
	vec3 diffuse = material.diffuseColor * ( 1.0 - max( max( totalScattering.r, totalScattering.g ), totalScattering.b ) );
	reflectedLight.indirectSpecular += radiance * singleScattering;
	reflectedLight.indirectSpecular += multiScattering * cosineWeightedIrradiance;
	reflectedLight.indirectDiffuse += diffuse * cosineWeightedIrradiance;
}
#define RE_Direct				RE_Direct_Physical
#define RE_Direct_RectArea		RE_Direct_RectArea_Physical
#define RE_IndirectDiffuse		RE_IndirectDiffuse_Physical
#define RE_IndirectSpecular		RE_IndirectSpecular_Physical
float computeSpecularOcclusion( const in float dotNV, const in float ambientOcclusion, const in float roughness ) {
	return saturate( pow( dotNV + ambientOcclusion, exp2( - 16.0 * roughness - 1.0 ) ) - 1.0 + ambientOcclusion );
}`,Kv=`
vec3 geometryPosition = - vViewPosition;
vec3 geometryNormal = normal;
vec3 geometryViewDir = ( isOrthographic ) ? vec3( 0, 0, 1 ) : normalize( vViewPosition );
vec3 geometryClearcoatNormal = vec3( 0.0 );
#ifdef USE_CLEARCOAT
	geometryClearcoatNormal = clearcoatNormal;
#endif
#ifdef USE_IRIDESCENCE
	float dotNVi = saturate( dot( normal, geometryViewDir ) );
	if ( material.iridescenceThickness == 0.0 ) {
		material.iridescence = 0.0;
	} else {
		material.iridescence = saturate( material.iridescence );
	}
	if ( material.iridescence > 0.0 ) {
		material.iridescenceFresnel = evalIridescence( 1.0, material.iridescenceIOR, dotNVi, material.iridescenceThickness, material.specularColor );
		material.iridescenceF0 = Schlick_to_F0( material.iridescenceFresnel, 1.0, dotNVi );
	}
#endif
IncidentLight directLight;
#if ( NUM_POINT_LIGHTS > 0 ) && defined( RE_Direct )
	PointLight pointLight;
	#if defined( USE_SHADOWMAP ) && NUM_POINT_LIGHT_SHADOWS > 0
	PointLightShadow pointLightShadow;
	#endif
	#pragma unroll_loop_start
	for ( int i = 0; i < NUM_POINT_LIGHTS; i ++ ) {
		pointLight = pointLights[ i ];
		getPointLightInfo( pointLight, geometryPosition, directLight );
		#if defined( USE_SHADOWMAP ) && ( UNROLLED_LOOP_INDEX < NUM_POINT_LIGHT_SHADOWS )
		pointLightShadow = pointLightShadows[ i ];
		directLight.color *= ( directLight.visible && receiveShadow ) ? getPointShadow( pointShadowMap[ i ], pointLightShadow.shadowMapSize, pointLightShadow.shadowBias, pointLightShadow.shadowRadius, vPointShadowCoord[ i ], pointLightShadow.shadowCameraNear, pointLightShadow.shadowCameraFar ) : 1.0;
		#endif
		RE_Direct( directLight, geometryPosition, geometryNormal, geometryViewDir, geometryClearcoatNormal, material, reflectedLight );
	}
	#pragma unroll_loop_end
#endif
#if ( NUM_SPOT_LIGHTS > 0 ) && defined( RE_Direct )
	SpotLight spotLight;
	vec4 spotColor;
	vec3 spotLightCoord;
	bool inSpotLightMap;
	#if defined( USE_SHADOWMAP ) && NUM_SPOT_LIGHT_SHADOWS > 0
	SpotLightShadow spotLightShadow;
	#endif
	#pragma unroll_loop_start
	for ( int i = 0; i < NUM_SPOT_LIGHTS; i ++ ) {
		spotLight = spotLights[ i ];
		getSpotLightInfo( spotLight, geometryPosition, directLight );
		#if ( UNROLLED_LOOP_INDEX < NUM_SPOT_LIGHT_SHADOWS_WITH_MAPS )
		#define SPOT_LIGHT_MAP_INDEX UNROLLED_LOOP_INDEX
		#elif ( UNROLLED_LOOP_INDEX < NUM_SPOT_LIGHT_SHADOWS )
		#define SPOT_LIGHT_MAP_INDEX NUM_SPOT_LIGHT_MAPS
		#else
		#define SPOT_LIGHT_MAP_INDEX ( UNROLLED_LOOP_INDEX - NUM_SPOT_LIGHT_SHADOWS + NUM_SPOT_LIGHT_SHADOWS_WITH_MAPS )
		#endif
		#if ( SPOT_LIGHT_MAP_INDEX < NUM_SPOT_LIGHT_MAPS )
			spotLightCoord = vSpotLightCoord[ i ].xyz / vSpotLightCoord[ i ].w;
			inSpotLightMap = all( lessThan( abs( spotLightCoord * 2. - 1. ), vec3( 1.0 ) ) );
			spotColor = texture2D( spotLightMap[ SPOT_LIGHT_MAP_INDEX ], spotLightCoord.xy );
			directLight.color = inSpotLightMap ? directLight.color * spotColor.rgb : directLight.color;
		#endif
		#undef SPOT_LIGHT_MAP_INDEX
		#if defined( USE_SHADOWMAP ) && ( UNROLLED_LOOP_INDEX < NUM_SPOT_LIGHT_SHADOWS )
		spotLightShadow = spotLightShadows[ i ];
		directLight.color *= ( directLight.visible && receiveShadow ) ? getShadow( spotShadowMap[ i ], spotLightShadow.shadowMapSize, spotLightShadow.shadowBias, spotLightShadow.shadowRadius, vSpotLightCoord[ i ] ) : 1.0;
		#endif
		RE_Direct( directLight, geometryPosition, geometryNormal, geometryViewDir, geometryClearcoatNormal, material, reflectedLight );
	}
	#pragma unroll_loop_end
#endif
#if ( NUM_DIR_LIGHTS > 0 ) && defined( RE_Direct )
	DirectionalLight directionalLight;
	#if defined( USE_SHADOWMAP ) && NUM_DIR_LIGHT_SHADOWS > 0
	DirectionalLightShadow directionalLightShadow;
	#endif
	#pragma unroll_loop_start
	for ( int i = 0; i < NUM_DIR_LIGHTS; i ++ ) {
		directionalLight = directionalLights[ i ];
		getDirectionalLightInfo( directionalLight, directLight );
		#if defined( USE_SHADOWMAP ) && ( UNROLLED_LOOP_INDEX < NUM_DIR_LIGHT_SHADOWS )
		directionalLightShadow = directionalLightShadows[ i ];
		directLight.color *= ( directLight.visible && receiveShadow ) ? getShadow( directionalShadowMap[ i ], directionalLightShadow.shadowMapSize, directionalLightShadow.shadowBias, directionalLightShadow.shadowRadius, vDirectionalShadowCoord[ i ] ) : 1.0;
		#endif
		RE_Direct( directLight, geometryPosition, geometryNormal, geometryViewDir, geometryClearcoatNormal, material, reflectedLight );
	}
	#pragma unroll_loop_end
#endif
#if ( NUM_RECT_AREA_LIGHTS > 0 ) && defined( RE_Direct_RectArea )
	RectAreaLight rectAreaLight;
	#pragma unroll_loop_start
	for ( int i = 0; i < NUM_RECT_AREA_LIGHTS; i ++ ) {
		rectAreaLight = rectAreaLights[ i ];
		RE_Direct_RectArea( rectAreaLight, geometryPosition, geometryNormal, geometryViewDir, geometryClearcoatNormal, material, reflectedLight );
	}
	#pragma unroll_loop_end
#endif
#if defined( RE_IndirectDiffuse )
	vec3 iblIrradiance = vec3( 0.0 );
	vec3 irradiance = getAmbientLightIrradiance( ambientLightColor );
	#if defined( USE_LIGHT_PROBES )
		irradiance += getLightProbeIrradiance( lightProbe, geometryNormal );
	#endif
	#if ( NUM_HEMI_LIGHTS > 0 )
		#pragma unroll_loop_start
		for ( int i = 0; i < NUM_HEMI_LIGHTS; i ++ ) {
			irradiance += getHemisphereLightIrradiance( hemisphereLights[ i ], geometryNormal );
		}
		#pragma unroll_loop_end
	#endif
#endif
#if defined( RE_IndirectSpecular )
	vec3 radiance = vec3( 0.0 );
	vec3 clearcoatRadiance = vec3( 0.0 );
#endif`,Qv=`#if defined( RE_IndirectDiffuse )
	#ifdef USE_LIGHTMAP
		vec4 lightMapTexel = texture2D( lightMap, vLightMapUv );
		vec3 lightMapIrradiance = lightMapTexel.rgb * lightMapIntensity;
		irradiance += lightMapIrradiance;
	#endif
	#if defined( USE_ENVMAP ) && defined( STANDARD ) && defined( ENVMAP_TYPE_CUBE_UV )
		iblIrradiance += getIBLIrradiance( geometryNormal );
	#endif
#endif
#if defined( USE_ENVMAP ) && defined( RE_IndirectSpecular )
	#ifdef USE_ANISOTROPY
		radiance += getIBLAnisotropyRadiance( geometryViewDir, geometryNormal, material.roughness, material.anisotropyB, material.anisotropy );
	#else
		radiance += getIBLRadiance( geometryViewDir, geometryNormal, material.roughness );
	#endif
	#ifdef USE_CLEARCOAT
		clearcoatRadiance += getIBLRadiance( geometryViewDir, geometryClearcoatNormal, material.clearcoatRoughness );
	#endif
#endif`,Zv=`#if defined( RE_IndirectDiffuse )
	RE_IndirectDiffuse( irradiance, geometryPosition, geometryNormal, geometryViewDir, geometryClearcoatNormal, material, reflectedLight );
#endif
#if defined( RE_IndirectSpecular )
	RE_IndirectSpecular( radiance, iblIrradiance, clearcoatRadiance, geometryPosition, geometryNormal, geometryViewDir, geometryClearcoatNormal, material, reflectedLight );
#endif`,Jv=`#if defined( USE_LOGDEPTHBUF ) && defined( USE_LOGDEPTHBUF_EXT )
	gl_FragDepthEXT = vIsPerspective == 0.0 ? gl_FragCoord.z : log2( vFragDepth ) * logDepthBufFC * 0.5;
#endif`,t0=`#if defined( USE_LOGDEPTHBUF ) && defined( USE_LOGDEPTHBUF_EXT )
	uniform float logDepthBufFC;
	varying float vFragDepth;
	varying float vIsPerspective;
#endif`,e0=`#ifdef USE_LOGDEPTHBUF
	#ifdef USE_LOGDEPTHBUF_EXT
		varying float vFragDepth;
		varying float vIsPerspective;
	#else
		uniform float logDepthBufFC;
	#endif
#endif`,n0=`#ifdef USE_LOGDEPTHBUF
	#ifdef USE_LOGDEPTHBUF_EXT
		vFragDepth = 1.0 + gl_Position.w;
		vIsPerspective = float( isPerspectiveMatrix( projectionMatrix ) );
	#else
		if ( isPerspectiveMatrix( projectionMatrix ) ) {
			gl_Position.z = log2( max( EPSILON, gl_Position.w + 1.0 ) ) * logDepthBufFC - 1.0;
			gl_Position.z *= gl_Position.w;
		}
	#endif
#endif`,i0=`#ifdef USE_MAP
	vec4 sampledDiffuseColor = texture2D( map, vMapUv );
	#ifdef DECODE_VIDEO_TEXTURE
		sampledDiffuseColor = vec4( mix( pow( sampledDiffuseColor.rgb * 0.9478672986 + vec3( 0.0521327014 ), vec3( 2.4 ) ), sampledDiffuseColor.rgb * 0.0773993808, vec3( lessThanEqual( sampledDiffuseColor.rgb, vec3( 0.04045 ) ) ) ), sampledDiffuseColor.w );
	
	#endif
	diffuseColor *= sampledDiffuseColor;
#endif`,r0=`#ifdef USE_MAP
	uniform sampler2D map;
#endif`,s0=`#if defined( USE_MAP ) || defined( USE_ALPHAMAP )
	#if defined( USE_POINTS_UV )
		vec2 uv = vUv;
	#else
		vec2 uv = ( uvTransform * vec3( gl_PointCoord.x, 1.0 - gl_PointCoord.y, 1 ) ).xy;
	#endif
#endif
#ifdef USE_MAP
	diffuseColor *= texture2D( map, uv );
#endif
#ifdef USE_ALPHAMAP
	diffuseColor.a *= texture2D( alphaMap, uv ).g;
#endif`,o0=`#if defined( USE_POINTS_UV )
	varying vec2 vUv;
#else
	#if defined( USE_MAP ) || defined( USE_ALPHAMAP )
		uniform mat3 uvTransform;
	#endif
#endif
#ifdef USE_MAP
	uniform sampler2D map;
#endif
#ifdef USE_ALPHAMAP
	uniform sampler2D alphaMap;
#endif`,a0=`float metalnessFactor = metalness;
#ifdef USE_METALNESSMAP
	vec4 texelMetalness = texture2D( metalnessMap, vMetalnessMapUv );
	metalnessFactor *= texelMetalness.b;
#endif`,c0=`#ifdef USE_METALNESSMAP
	uniform sampler2D metalnessMap;
#endif`,l0=`#if defined( USE_MORPHCOLORS ) && defined( MORPHTARGETS_TEXTURE )
	vColor *= morphTargetBaseInfluence;
	for ( int i = 0; i < MORPHTARGETS_COUNT; i ++ ) {
		#if defined( USE_COLOR_ALPHA )
			if ( morphTargetInfluences[ i ] != 0.0 ) vColor += getMorph( gl_VertexID, i, 2 ) * morphTargetInfluences[ i ];
		#elif defined( USE_COLOR )
			if ( morphTargetInfluences[ i ] != 0.0 ) vColor += getMorph( gl_VertexID, i, 2 ).rgb * morphTargetInfluences[ i ];
		#endif
	}
#endif`,h0=`#ifdef USE_MORPHNORMALS
	objectNormal *= morphTargetBaseInfluence;
	#ifdef MORPHTARGETS_TEXTURE
		for ( int i = 0; i < MORPHTARGETS_COUNT; i ++ ) {
			if ( morphTargetInfluences[ i ] != 0.0 ) objectNormal += getMorph( gl_VertexID, i, 1 ).xyz * morphTargetInfluences[ i ];
		}
	#else
		objectNormal += morphNormal0 * morphTargetInfluences[ 0 ];
		objectNormal += morphNormal1 * morphTargetInfluences[ 1 ];
		objectNormal += morphNormal2 * morphTargetInfluences[ 2 ];
		objectNormal += morphNormal3 * morphTargetInfluences[ 3 ];
	#endif
#endif`,u0=`#ifdef USE_MORPHTARGETS
	uniform float morphTargetBaseInfluence;
	#ifdef MORPHTARGETS_TEXTURE
		uniform float morphTargetInfluences[ MORPHTARGETS_COUNT ];
		uniform sampler2DArray morphTargetsTexture;
		uniform ivec2 morphTargetsTextureSize;
		vec4 getMorph( const in int vertexIndex, const in int morphTargetIndex, const in int offset ) {
			int texelIndex = vertexIndex * MORPHTARGETS_TEXTURE_STRIDE + offset;
			int y = texelIndex / morphTargetsTextureSize.x;
			int x = texelIndex - y * morphTargetsTextureSize.x;
			ivec3 morphUV = ivec3( x, y, morphTargetIndex );
			return texelFetch( morphTargetsTexture, morphUV, 0 );
		}
	#else
		#ifndef USE_MORPHNORMALS
			uniform float morphTargetInfluences[ 8 ];
		#else
			uniform float morphTargetInfluences[ 4 ];
		#endif
	#endif
#endif`,d0=`#ifdef USE_MORPHTARGETS
	transformed *= morphTargetBaseInfluence;
	#ifdef MORPHTARGETS_TEXTURE
		for ( int i = 0; i < MORPHTARGETS_COUNT; i ++ ) {
			if ( morphTargetInfluences[ i ] != 0.0 ) transformed += getMorph( gl_VertexID, i, 0 ).xyz * morphTargetInfluences[ i ];
		}
	#else
		transformed += morphTarget0 * morphTargetInfluences[ 0 ];
		transformed += morphTarget1 * morphTargetInfluences[ 1 ];
		transformed += morphTarget2 * morphTargetInfluences[ 2 ];
		transformed += morphTarget3 * morphTargetInfluences[ 3 ];
		#ifndef USE_MORPHNORMALS
			transformed += morphTarget4 * morphTargetInfluences[ 4 ];
			transformed += morphTarget5 * morphTargetInfluences[ 5 ];
			transformed += morphTarget6 * morphTargetInfluences[ 6 ];
			transformed += morphTarget7 * morphTargetInfluences[ 7 ];
		#endif
	#endif
#endif`,f0=`float faceDirection = gl_FrontFacing ? 1.0 : - 1.0;
#ifdef FLAT_SHADED
	vec3 fdx = dFdx( vViewPosition );
	vec3 fdy = dFdy( vViewPosition );
	vec3 normal = normalize( cross( fdx, fdy ) );
#else
	vec3 normal = normalize( vNormal );
	#ifdef DOUBLE_SIDED
		normal *= faceDirection;
	#endif
#endif
#if defined( USE_NORMALMAP_TANGENTSPACE ) || defined( USE_CLEARCOAT_NORMALMAP ) || defined( USE_ANISOTROPY )
	#ifdef USE_TANGENT
		mat3 tbn = mat3( normalize( vTangent ), normalize( vBitangent ), normal );
	#else
		mat3 tbn = getTangentFrame( - vViewPosition, normal,
		#if defined( USE_NORMALMAP )
			vNormalMapUv
		#elif defined( USE_CLEARCOAT_NORMALMAP )
			vClearcoatNormalMapUv
		#else
			vUv
		#endif
		);
	#endif
	#if defined( DOUBLE_SIDED ) && ! defined( FLAT_SHADED )
		tbn[0] *= faceDirection;
		tbn[1] *= faceDirection;
	#endif
#endif
#ifdef USE_CLEARCOAT_NORMALMAP
	#ifdef USE_TANGENT
		mat3 tbn2 = mat3( normalize( vTangent ), normalize( vBitangent ), normal );
	#else
		mat3 tbn2 = getTangentFrame( - vViewPosition, normal, vClearcoatNormalMapUv );
	#endif
	#if defined( DOUBLE_SIDED ) && ! defined( FLAT_SHADED )
		tbn2[0] *= faceDirection;
		tbn2[1] *= faceDirection;
	#endif
#endif
vec3 nonPerturbedNormal = normal;`,p0=`#ifdef USE_NORMALMAP_OBJECTSPACE
	normal = texture2D( normalMap, vNormalMapUv ).xyz * 2.0 - 1.0;
	#ifdef FLIP_SIDED
		normal = - normal;
	#endif
	#ifdef DOUBLE_SIDED
		normal = normal * faceDirection;
	#endif
	normal = normalize( normalMatrix * normal );
#elif defined( USE_NORMALMAP_TANGENTSPACE )
	vec3 mapN = texture2D( normalMap, vNormalMapUv ).xyz * 2.0 - 1.0;
	mapN.xy *= normalScale;
	normal = normalize( tbn * mapN );
#elif defined( USE_BUMPMAP )
	normal = perturbNormalArb( - vViewPosition, normal, dHdxy_fwd(), faceDirection );
#endif`,m0=`#ifndef FLAT_SHADED
	varying vec3 vNormal;
	#ifdef USE_TANGENT
		varying vec3 vTangent;
		varying vec3 vBitangent;
	#endif
#endif`,g0=`#ifndef FLAT_SHADED
	varying vec3 vNormal;
	#ifdef USE_TANGENT
		varying vec3 vTangent;
		varying vec3 vBitangent;
	#endif
#endif`,_0=`#ifndef FLAT_SHADED
	vNormal = normalize( transformedNormal );
	#ifdef USE_TANGENT
		vTangent = normalize( transformedTangent );
		vBitangent = normalize( cross( vNormal, vTangent ) * tangent.w );
	#endif
#endif`,v0=`#ifdef USE_NORMALMAP
	uniform sampler2D normalMap;
	uniform vec2 normalScale;
#endif
#ifdef USE_NORMALMAP_OBJECTSPACE
	uniform mat3 normalMatrix;
#endif
#if ! defined ( USE_TANGENT ) && ( defined ( USE_NORMALMAP_TANGENTSPACE ) || defined ( USE_CLEARCOAT_NORMALMAP ) || defined( USE_ANISOTROPY ) )
	mat3 getTangentFrame( vec3 eye_pos, vec3 surf_norm, vec2 uv ) {
		vec3 q0 = dFdx( eye_pos.xyz );
		vec3 q1 = dFdy( eye_pos.xyz );
		vec2 st0 = dFdx( uv.st );
		vec2 st1 = dFdy( uv.st );
		vec3 N = surf_norm;
		vec3 q1perp = cross( q1, N );
		vec3 q0perp = cross( N, q0 );
		vec3 T = q1perp * st0.x + q0perp * st1.x;
		vec3 B = q1perp * st0.y + q0perp * st1.y;
		float det = max( dot( T, T ), dot( B, B ) );
		float scale = ( det == 0.0 ) ? 0.0 : inversesqrt( det );
		return mat3( T * scale, B * scale, N );
	}
#endif`,y0=`#ifdef USE_CLEARCOAT
	vec3 clearcoatNormal = nonPerturbedNormal;
#endif`,E0=`#ifdef USE_CLEARCOAT_NORMALMAP
	vec3 clearcoatMapN = texture2D( clearcoatNormalMap, vClearcoatNormalMapUv ).xyz * 2.0 - 1.0;
	clearcoatMapN.xy *= clearcoatNormalScale;
	clearcoatNormal = normalize( tbn2 * clearcoatMapN );
#endif`,x0=`#ifdef USE_CLEARCOATMAP
	uniform sampler2D clearcoatMap;
#endif
#ifdef USE_CLEARCOAT_NORMALMAP
	uniform sampler2D clearcoatNormalMap;
	uniform vec2 clearcoatNormalScale;
#endif
#ifdef USE_CLEARCOAT_ROUGHNESSMAP
	uniform sampler2D clearcoatRoughnessMap;
#endif`,S0=`#ifdef USE_IRIDESCENCEMAP
	uniform sampler2D iridescenceMap;
#endif
#ifdef USE_IRIDESCENCE_THICKNESSMAP
	uniform sampler2D iridescenceThicknessMap;
#endif`,T0=`#ifdef OPAQUE
diffuseColor.a = 1.0;
#endif
#ifdef USE_TRANSMISSION
diffuseColor.a *= material.transmissionAlpha;
#endif
gl_FragColor = vec4( outgoingLight, diffuseColor.a );`,M0=`vec3 packNormalToRGB( const in vec3 normal ) {
	return normalize( normal ) * 0.5 + 0.5;
}
vec3 unpackRGBToNormal( const in vec3 rgb ) {
	return 2.0 * rgb.xyz - 1.0;
}
const float PackUpscale = 256. / 255.;const float UnpackDownscale = 255. / 256.;
const vec3 PackFactors = vec3( 256. * 256. * 256., 256. * 256., 256. );
const vec4 UnpackFactors = UnpackDownscale / vec4( PackFactors, 1. );
const float ShiftRight8 = 1. / 256.;
vec4 packDepthToRGBA( const in float v ) {
	vec4 r = vec4( fract( v * PackFactors ), v );
	r.yzw -= r.xyz * ShiftRight8;	return r * PackUpscale;
}
float unpackRGBAToDepth( const in vec4 v ) {
	return dot( v, UnpackFactors );
}
vec2 packDepthToRG( in highp float v ) {
	return packDepthToRGBA( v ).yx;
}
float unpackRGToDepth( const in highp vec2 v ) {
	return unpackRGBAToDepth( vec4( v.xy, 0.0, 0.0 ) );
}
vec4 pack2HalfToRGBA( vec2 v ) {
	vec4 r = vec4( v.x, fract( v.x * 255.0 ), v.y, fract( v.y * 255.0 ) );
	return vec4( r.x - r.y / 255.0, r.y, r.z - r.w / 255.0, r.w );
}
vec2 unpackRGBATo2Half( vec4 v ) {
	return vec2( v.x + ( v.y / 255.0 ), v.z + ( v.w / 255.0 ) );
}
float viewZToOrthographicDepth( const in float viewZ, const in float near, const in float far ) {
	return ( viewZ + near ) / ( near - far );
}
float orthographicDepthToViewZ( const in float depth, const in float near, const in float far ) {
	return depth * ( near - far ) - near;
}
float viewZToPerspectiveDepth( const in float viewZ, const in float near, const in float far ) {
	return ( ( near + viewZ ) * far ) / ( ( far - near ) * viewZ );
}
float perspectiveDepthToViewZ( const in float depth, const in float near, const in float far ) {
	return ( near * far ) / ( ( far - near ) * depth - far );
}`,w0=`#ifdef PREMULTIPLIED_ALPHA
	gl_FragColor.rgb *= gl_FragColor.a;
#endif`,b0=`vec4 mvPosition = vec4( transformed, 1.0 );
#ifdef USE_BATCHING
	mvPosition = batchingMatrix * mvPosition;
#endif
#ifdef USE_INSTANCING
	mvPosition = instanceMatrix * mvPosition;
#endif
mvPosition = modelViewMatrix * mvPosition;
gl_Position = projectionMatrix * mvPosition;`,A0=`#ifdef DITHERING
	gl_FragColor.rgb = dithering( gl_FragColor.rgb );
#endif`,R0=`#ifdef DITHERING
	vec3 dithering( vec3 color ) {
		float grid_position = rand( gl_FragCoord.xy );
		vec3 dither_shift_RGB = vec3( 0.25 / 255.0, -0.25 / 255.0, 0.25 / 255.0 );
		dither_shift_RGB = mix( 2.0 * dither_shift_RGB, -2.0 * dither_shift_RGB, grid_position );
		return color + dither_shift_RGB;
	}
#endif`,C0=`float roughnessFactor = roughness;
#ifdef USE_ROUGHNESSMAP
	vec4 texelRoughness = texture2D( roughnessMap, vRoughnessMapUv );
	roughnessFactor *= texelRoughness.g;
#endif`,I0=`#ifdef USE_ROUGHNESSMAP
	uniform sampler2D roughnessMap;
#endif`,P0=`#if NUM_SPOT_LIGHT_COORDS > 0
	varying vec4 vSpotLightCoord[ NUM_SPOT_LIGHT_COORDS ];
#endif
#if NUM_SPOT_LIGHT_MAPS > 0
	uniform sampler2D spotLightMap[ NUM_SPOT_LIGHT_MAPS ];
#endif
#ifdef USE_SHADOWMAP
	#if NUM_DIR_LIGHT_SHADOWS > 0
		uniform sampler2D directionalShadowMap[ NUM_DIR_LIGHT_SHADOWS ];
		varying vec4 vDirectionalShadowCoord[ NUM_DIR_LIGHT_SHADOWS ];
		struct DirectionalLightShadow {
			float shadowBias;
			float shadowNormalBias;
			float shadowRadius;
			vec2 shadowMapSize;
		};
		uniform DirectionalLightShadow directionalLightShadows[ NUM_DIR_LIGHT_SHADOWS ];
	#endif
	#if NUM_SPOT_LIGHT_SHADOWS > 0
		uniform sampler2D spotShadowMap[ NUM_SPOT_LIGHT_SHADOWS ];
		struct SpotLightShadow {
			float shadowBias;
			float shadowNormalBias;
			float shadowRadius;
			vec2 shadowMapSize;
		};
		uniform SpotLightShadow spotLightShadows[ NUM_SPOT_LIGHT_SHADOWS ];
	#endif
	#if NUM_POINT_LIGHT_SHADOWS > 0
		uniform sampler2D pointShadowMap[ NUM_POINT_LIGHT_SHADOWS ];
		varying vec4 vPointShadowCoord[ NUM_POINT_LIGHT_SHADOWS ];
		struct PointLightShadow {
			float shadowBias;
			float shadowNormalBias;
			float shadowRadius;
			vec2 shadowMapSize;
			float shadowCameraNear;
			float shadowCameraFar;
		};
		uniform PointLightShadow pointLightShadows[ NUM_POINT_LIGHT_SHADOWS ];
	#endif
	float texture2DCompare( sampler2D depths, vec2 uv, float compare ) {
		return step( compare, unpackRGBAToDepth( texture2D( depths, uv ) ) );
	}
	vec2 texture2DDistribution( sampler2D shadow, vec2 uv ) {
		return unpackRGBATo2Half( texture2D( shadow, uv ) );
	}
	float VSMShadow (sampler2D shadow, vec2 uv, float compare ){
		float occlusion = 1.0;
		vec2 distribution = texture2DDistribution( shadow, uv );
		float hard_shadow = step( compare , distribution.x );
		if (hard_shadow != 1.0 ) {
			float distance = compare - distribution.x ;
			float variance = max( 0.00000, distribution.y * distribution.y );
			float softness_probability = variance / (variance + distance * distance );			softness_probability = clamp( ( softness_probability - 0.3 ) / ( 0.95 - 0.3 ), 0.0, 1.0 );			occlusion = clamp( max( hard_shadow, softness_probability ), 0.0, 1.0 );
		}
		return occlusion;
	}
	float getShadow( sampler2D shadowMap, vec2 shadowMapSize, float shadowBias, float shadowRadius, vec4 shadowCoord ) {
		float shadow = 1.0;
		shadowCoord.xyz /= shadowCoord.w;
		shadowCoord.z += shadowBias;
		bool inFrustum = shadowCoord.x >= 0.0 && shadowCoord.x <= 1.0 && shadowCoord.y >= 0.0 && shadowCoord.y <= 1.0;
		bool frustumTest = inFrustum && shadowCoord.z <= 1.0;
		if ( frustumTest ) {
		#if defined( SHADOWMAP_TYPE_PCF )
			vec2 texelSize = vec2( 1.0 ) / shadowMapSize;
			float dx0 = - texelSize.x * shadowRadius;
			float dy0 = - texelSize.y * shadowRadius;
			float dx1 = + texelSize.x * shadowRadius;
			float dy1 = + texelSize.y * shadowRadius;
			float dx2 = dx0 / 2.0;
			float dy2 = dy0 / 2.0;
			float dx3 = dx1 / 2.0;
			float dy3 = dy1 / 2.0;
			shadow = (
				texture2DCompare( shadowMap, shadowCoord.xy + vec2( dx0, dy0 ), shadowCoord.z ) +
				texture2DCompare( shadowMap, shadowCoord.xy + vec2( 0.0, dy0 ), shadowCoord.z ) +
				texture2DCompare( shadowMap, shadowCoord.xy + vec2( dx1, dy0 ), shadowCoord.z ) +
				texture2DCompare( shadowMap, shadowCoord.xy + vec2( dx2, dy2 ), shadowCoord.z ) +
				texture2DCompare( shadowMap, shadowCoord.xy + vec2( 0.0, dy2 ), shadowCoord.z ) +
				texture2DCompare( shadowMap, shadowCoord.xy + vec2( dx3, dy2 ), shadowCoord.z ) +
				texture2DCompare( shadowMap, shadowCoord.xy + vec2( dx0, 0.0 ), shadowCoord.z ) +
				texture2DCompare( shadowMap, shadowCoord.xy + vec2( dx2, 0.0 ), shadowCoord.z ) +
				texture2DCompare( shadowMap, shadowCoord.xy, shadowCoord.z ) +
				texture2DCompare( shadowMap, shadowCoord.xy + vec2( dx3, 0.0 ), shadowCoord.z ) +
				texture2DCompare( shadowMap, shadowCoord.xy + vec2( dx1, 0.0 ), shadowCoord.z ) +
				texture2DCompare( shadowMap, shadowCoord.xy + vec2( dx2, dy3 ), shadowCoord.z ) +
				texture2DCompare( shadowMap, shadowCoord.xy + vec2( 0.0, dy3 ), shadowCoord.z ) +
				texture2DCompare( shadowMap, shadowCoord.xy + vec2( dx3, dy3 ), shadowCoord.z ) +
				texture2DCompare( shadowMap, shadowCoord.xy + vec2( dx0, dy1 ), shadowCoord.z ) +
				texture2DCompare( shadowMap, shadowCoord.xy + vec2( 0.0, dy1 ), shadowCoord.z ) +
				texture2DCompare( shadowMap, shadowCoord.xy + vec2( dx1, dy1 ), shadowCoord.z )
			) * ( 1.0 / 17.0 );
		#elif defined( SHADOWMAP_TYPE_PCF_SOFT )
			vec2 texelSize = vec2( 1.0 ) / shadowMapSize;
			float dx = texelSize.x;
			float dy = texelSize.y;
			vec2 uv = shadowCoord.xy;
			vec2 f = fract( uv * shadowMapSize + 0.5 );
			uv -= f * texelSize;
			shadow = (
				texture2DCompare( shadowMap, uv, shadowCoord.z ) +
				texture2DCompare( shadowMap, uv + vec2( dx, 0.0 ), shadowCoord.z ) +
				texture2DCompare( shadowMap, uv + vec2( 0.0, dy ), shadowCoord.z ) +
				texture2DCompare( shadowMap, uv + texelSize, shadowCoord.z ) +
				mix( texture2DCompare( shadowMap, uv + vec2( -dx, 0.0 ), shadowCoord.z ),
					 texture2DCompare( shadowMap, uv + vec2( 2.0 * dx, 0.0 ), shadowCoord.z ),
					 f.x ) +
				mix( texture2DCompare( shadowMap, uv + vec2( -dx, dy ), shadowCoord.z ),
					 texture2DCompare( shadowMap, uv + vec2( 2.0 * dx, dy ), shadowCoord.z ),
					 f.x ) +
				mix( texture2DCompare( shadowMap, uv + vec2( 0.0, -dy ), shadowCoord.z ),
					 texture2DCompare( shadowMap, uv + vec2( 0.0, 2.0 * dy ), shadowCoord.z ),
					 f.y ) +
				mix( texture2DCompare( shadowMap, uv + vec2( dx, -dy ), shadowCoord.z ),
					 texture2DCompare( shadowMap, uv + vec2( dx, 2.0 * dy ), shadowCoord.z ),
					 f.y ) +
				mix( mix( texture2DCompare( shadowMap, uv + vec2( -dx, -dy ), shadowCoord.z ),
						  texture2DCompare( shadowMap, uv + vec2( 2.0 * dx, -dy ), shadowCoord.z ),
						  f.x ),
					 mix( texture2DCompare( shadowMap, uv + vec2( -dx, 2.0 * dy ), shadowCoord.z ),
						  texture2DCompare( shadowMap, uv + vec2( 2.0 * dx, 2.0 * dy ), shadowCoord.z ),
						  f.x ),
					 f.y )
			) * ( 1.0 / 9.0 );
		#elif defined( SHADOWMAP_TYPE_VSM )
			shadow = VSMShadow( shadowMap, shadowCoord.xy, shadowCoord.z );
		#else
			shadow = texture2DCompare( shadowMap, shadowCoord.xy, shadowCoord.z );
		#endif
		}
		return shadow;
	}
	vec2 cubeToUV( vec3 v, float texelSizeY ) {
		vec3 absV = abs( v );
		float scaleToCube = 1.0 / max( absV.x, max( absV.y, absV.z ) );
		absV *= scaleToCube;
		v *= scaleToCube * ( 1.0 - 2.0 * texelSizeY );
		vec2 planar = v.xy;
		float almostATexel = 1.5 * texelSizeY;
		float almostOne = 1.0 - almostATexel;
		if ( absV.z >= almostOne ) {
			if ( v.z > 0.0 )
				planar.x = 4.0 - v.x;
		} else if ( absV.x >= almostOne ) {
			float signX = sign( v.x );
			planar.x = v.z * signX + 2.0 * signX;
		} else if ( absV.y >= almostOne ) {
			float signY = sign( v.y );
			planar.x = v.x + 2.0 * signY + 2.0;
			planar.y = v.z * signY - 2.0;
		}
		return vec2( 0.125, 0.25 ) * planar + vec2( 0.375, 0.75 );
	}
	float getPointShadow( sampler2D shadowMap, vec2 shadowMapSize, float shadowBias, float shadowRadius, vec4 shadowCoord, float shadowCameraNear, float shadowCameraFar ) {
		vec2 texelSize = vec2( 1.0 ) / ( shadowMapSize * vec2( 4.0, 2.0 ) );
		vec3 lightToPosition = shadowCoord.xyz;
		float dp = ( length( lightToPosition ) - shadowCameraNear ) / ( shadowCameraFar - shadowCameraNear );		dp += shadowBias;
		vec3 bd3D = normalize( lightToPosition );
		#if defined( SHADOWMAP_TYPE_PCF ) || defined( SHADOWMAP_TYPE_PCF_SOFT ) || defined( SHADOWMAP_TYPE_VSM )
			vec2 offset = vec2( - 1, 1 ) * shadowRadius * texelSize.y;
			return (
				texture2DCompare( shadowMap, cubeToUV( bd3D + offset.xyy, texelSize.y ), dp ) +
				texture2DCompare( shadowMap, cubeToUV( bd3D + offset.yyy, texelSize.y ), dp ) +
				texture2DCompare( shadowMap, cubeToUV( bd3D + offset.xyx, texelSize.y ), dp ) +
				texture2DCompare( shadowMap, cubeToUV( bd3D + offset.yyx, texelSize.y ), dp ) +
				texture2DCompare( shadowMap, cubeToUV( bd3D, texelSize.y ), dp ) +
				texture2DCompare( shadowMap, cubeToUV( bd3D + offset.xxy, texelSize.y ), dp ) +
				texture2DCompare( shadowMap, cubeToUV( bd3D + offset.yxy, texelSize.y ), dp ) +
				texture2DCompare( shadowMap, cubeToUV( bd3D + offset.xxx, texelSize.y ), dp ) +
				texture2DCompare( shadowMap, cubeToUV( bd3D + offset.yxx, texelSize.y ), dp )
			) * ( 1.0 / 9.0 );
		#else
			return texture2DCompare( shadowMap, cubeToUV( bd3D, texelSize.y ), dp );
		#endif
	}
#endif`,D0=`#if NUM_SPOT_LIGHT_COORDS > 0
	uniform mat4 spotLightMatrix[ NUM_SPOT_LIGHT_COORDS ];
	varying vec4 vSpotLightCoord[ NUM_SPOT_LIGHT_COORDS ];
#endif
#ifdef USE_SHADOWMAP
	#if NUM_DIR_LIGHT_SHADOWS > 0
		uniform mat4 directionalShadowMatrix[ NUM_DIR_LIGHT_SHADOWS ];
		varying vec4 vDirectionalShadowCoord[ NUM_DIR_LIGHT_SHADOWS ];
		struct DirectionalLightShadow {
			float shadowBias;
			float shadowNormalBias;
			float shadowRadius;
			vec2 shadowMapSize;
		};
		uniform DirectionalLightShadow directionalLightShadows[ NUM_DIR_LIGHT_SHADOWS ];
	#endif
	#if NUM_SPOT_LIGHT_SHADOWS > 0
		struct SpotLightShadow {
			float shadowBias;
			float shadowNormalBias;
			float shadowRadius;
			vec2 shadowMapSize;
		};
		uniform SpotLightShadow spotLightShadows[ NUM_SPOT_LIGHT_SHADOWS ];
	#endif
	#if NUM_POINT_LIGHT_SHADOWS > 0
		uniform mat4 pointShadowMatrix[ NUM_POINT_LIGHT_SHADOWS ];
		varying vec4 vPointShadowCoord[ NUM_POINT_LIGHT_SHADOWS ];
		struct PointLightShadow {
			float shadowBias;
			float shadowNormalBias;
			float shadowRadius;
			vec2 shadowMapSize;
			float shadowCameraNear;
			float shadowCameraFar;
		};
		uniform PointLightShadow pointLightShadows[ NUM_POINT_LIGHT_SHADOWS ];
	#endif
#endif`,L0=`#if ( defined( USE_SHADOWMAP ) && ( NUM_DIR_LIGHT_SHADOWS > 0 || NUM_POINT_LIGHT_SHADOWS > 0 ) ) || ( NUM_SPOT_LIGHT_COORDS > 0 )
	vec3 shadowWorldNormal = inverseTransformDirection( transformedNormal, viewMatrix );
	vec4 shadowWorldPosition;
#endif
#if defined( USE_SHADOWMAP )
	#if NUM_DIR_LIGHT_SHADOWS > 0
		#pragma unroll_loop_start
		for ( int i = 0; i < NUM_DIR_LIGHT_SHADOWS; i ++ ) {
			shadowWorldPosition = worldPosition + vec4( shadowWorldNormal * directionalLightShadows[ i ].shadowNormalBias, 0 );
			vDirectionalShadowCoord[ i ] = directionalShadowMatrix[ i ] * shadowWorldPosition;
		}
		#pragma unroll_loop_end
	#endif
	#if NUM_POINT_LIGHT_SHADOWS > 0
		#pragma unroll_loop_start
		for ( int i = 0; i < NUM_POINT_LIGHT_SHADOWS; i ++ ) {
			shadowWorldPosition = worldPosition + vec4( shadowWorldNormal * pointLightShadows[ i ].shadowNormalBias, 0 );
			vPointShadowCoord[ i ] = pointShadowMatrix[ i ] * shadowWorldPosition;
		}
		#pragma unroll_loop_end
	#endif
#endif
#if NUM_SPOT_LIGHT_COORDS > 0
	#pragma unroll_loop_start
	for ( int i = 0; i < NUM_SPOT_LIGHT_COORDS; i ++ ) {
		shadowWorldPosition = worldPosition;
		#if ( defined( USE_SHADOWMAP ) && UNROLLED_LOOP_INDEX < NUM_SPOT_LIGHT_SHADOWS )
			shadowWorldPosition.xyz += shadowWorldNormal * spotLightShadows[ i ].shadowNormalBias;
		#endif
		vSpotLightCoord[ i ] = spotLightMatrix[ i ] * shadowWorldPosition;
	}
	#pragma unroll_loop_end
#endif`,N0=`float getShadowMask() {
	float shadow = 1.0;
	#ifdef USE_SHADOWMAP
	#if NUM_DIR_LIGHT_SHADOWS > 0
	DirectionalLightShadow directionalLight;
	#pragma unroll_loop_start
	for ( int i = 0; i < NUM_DIR_LIGHT_SHADOWS; i ++ ) {
		directionalLight = directionalLightShadows[ i ];
		shadow *= receiveShadow ? getShadow( directionalShadowMap[ i ], directionalLight.shadowMapSize, directionalLight.shadowBias, directionalLight.shadowRadius, vDirectionalShadowCoord[ i ] ) : 1.0;
	}
	#pragma unroll_loop_end
	#endif
	#if NUM_SPOT_LIGHT_SHADOWS > 0
	SpotLightShadow spotLight;
	#pragma unroll_loop_start
	for ( int i = 0; i < NUM_SPOT_LIGHT_SHADOWS; i ++ ) {
		spotLight = spotLightShadows[ i ];
		shadow *= receiveShadow ? getShadow( spotShadowMap[ i ], spotLight.shadowMapSize, spotLight.shadowBias, spotLight.shadowRadius, vSpotLightCoord[ i ] ) : 1.0;
	}
	#pragma unroll_loop_end
	#endif
	#if NUM_POINT_LIGHT_SHADOWS > 0
	PointLightShadow pointLight;
	#pragma unroll_loop_start
	for ( int i = 0; i < NUM_POINT_LIGHT_SHADOWS; i ++ ) {
		pointLight = pointLightShadows[ i ];
		shadow *= receiveShadow ? getPointShadow( pointShadowMap[ i ], pointLight.shadowMapSize, pointLight.shadowBias, pointLight.shadowRadius, vPointShadowCoord[ i ], pointLight.shadowCameraNear, pointLight.shadowCameraFar ) : 1.0;
	}
	#pragma unroll_loop_end
	#endif
	#endif
	return shadow;
}`,U0=`#ifdef USE_SKINNING
	mat4 boneMatX = getBoneMatrix( skinIndex.x );
	mat4 boneMatY = getBoneMatrix( skinIndex.y );
	mat4 boneMatZ = getBoneMatrix( skinIndex.z );
	mat4 boneMatW = getBoneMatrix( skinIndex.w );
#endif`,O0=`#ifdef USE_SKINNING
	uniform mat4 bindMatrix;
	uniform mat4 bindMatrixInverse;
	uniform highp sampler2D boneTexture;
	mat4 getBoneMatrix( const in float i ) {
		int size = textureSize( boneTexture, 0 ).x;
		int j = int( i ) * 4;
		int x = j % size;
		int y = j / size;
		vec4 v1 = texelFetch( boneTexture, ivec2( x, y ), 0 );
		vec4 v2 = texelFetch( boneTexture, ivec2( x + 1, y ), 0 );
		vec4 v3 = texelFetch( boneTexture, ivec2( x + 2, y ), 0 );
		vec4 v4 = texelFetch( boneTexture, ivec2( x + 3, y ), 0 );
		return mat4( v1, v2, v3, v4 );
	}
#endif`,F0=`#ifdef USE_SKINNING
	vec4 skinVertex = bindMatrix * vec4( transformed, 1.0 );
	vec4 skinned = vec4( 0.0 );
	skinned += boneMatX * skinVertex * skinWeight.x;
	skinned += boneMatY * skinVertex * skinWeight.y;
	skinned += boneMatZ * skinVertex * skinWeight.z;
	skinned += boneMatW * skinVertex * skinWeight.w;
	transformed = ( bindMatrixInverse * skinned ).xyz;
#endif`,V0=`#ifdef USE_SKINNING
	mat4 skinMatrix = mat4( 0.0 );
	skinMatrix += skinWeight.x * boneMatX;
	skinMatrix += skinWeight.y * boneMatY;
	skinMatrix += skinWeight.z * boneMatZ;
	skinMatrix += skinWeight.w * boneMatW;
	skinMatrix = bindMatrixInverse * skinMatrix * bindMatrix;
	objectNormal = vec4( skinMatrix * vec4( objectNormal, 0.0 ) ).xyz;
	#ifdef USE_TANGENT
		objectTangent = vec4( skinMatrix * vec4( objectTangent, 0.0 ) ).xyz;
	#endif
#endif`,B0=`float specularStrength;
#ifdef USE_SPECULARMAP
	vec4 texelSpecular = texture2D( specularMap, vSpecularMapUv );
	specularStrength = texelSpecular.r;
#else
	specularStrength = 1.0;
#endif`,k0=`#ifdef USE_SPECULARMAP
	uniform sampler2D specularMap;
#endif`,H0=`#if defined( TONE_MAPPING )
	gl_FragColor.rgb = toneMapping( gl_FragColor.rgb );
#endif`,z0=`#ifndef saturate
#define saturate( a ) clamp( a, 0.0, 1.0 )
#endif
uniform float toneMappingExposure;
vec3 LinearToneMapping( vec3 color ) {
	return saturate( toneMappingExposure * color );
}
vec3 ReinhardToneMapping( vec3 color ) {
	color *= toneMappingExposure;
	return saturate( color / ( vec3( 1.0 ) + color ) );
}
vec3 OptimizedCineonToneMapping( vec3 color ) {
	color *= toneMappingExposure;
	color = max( vec3( 0.0 ), color - 0.004 );
	return pow( ( color * ( 6.2 * color + 0.5 ) ) / ( color * ( 6.2 * color + 1.7 ) + 0.06 ), vec3( 2.2 ) );
}
vec3 RRTAndODTFit( vec3 v ) {
	vec3 a = v * ( v + 0.0245786 ) - 0.000090537;
	vec3 b = v * ( 0.983729 * v + 0.4329510 ) + 0.238081;
	return a / b;
}
vec3 ACESFilmicToneMapping( vec3 color ) {
	const mat3 ACESInputMat = mat3(
		vec3( 0.59719, 0.07600, 0.02840 ),		vec3( 0.35458, 0.90834, 0.13383 ),
		vec3( 0.04823, 0.01566, 0.83777 )
	);
	const mat3 ACESOutputMat = mat3(
		vec3(  1.60475, -0.10208, -0.00327 ),		vec3( -0.53108,  1.10813, -0.07276 ),
		vec3( -0.07367, -0.00605,  1.07602 )
	);
	color *= toneMappingExposure / 0.6;
	color = ACESInputMat * color;
	color = RRTAndODTFit( color );
	color = ACESOutputMat * color;
	return saturate( color );
}
const mat3 LINEAR_REC2020_TO_LINEAR_SRGB = mat3(
	vec3( 1.6605, - 0.1246, - 0.0182 ),
	vec3( - 0.5876, 1.1329, - 0.1006 ),
	vec3( - 0.0728, - 0.0083, 1.1187 )
);
const mat3 LINEAR_SRGB_TO_LINEAR_REC2020 = mat3(
	vec3( 0.6274, 0.0691, 0.0164 ),
	vec3( 0.3293, 0.9195, 0.0880 ),
	vec3( 0.0433, 0.0113, 0.8956 )
);
vec3 agxDefaultContrastApprox( vec3 x ) {
	vec3 x2 = x * x;
	vec3 x4 = x2 * x2;
	return + 15.5 * x4 * x2
		- 40.14 * x4 * x
		+ 31.96 * x4
		- 6.868 * x2 * x
		+ 0.4298 * x2
		+ 0.1191 * x
		- 0.00232;
}
vec3 AgXToneMapping( vec3 color ) {
	const mat3 AgXInsetMatrix = mat3(
		vec3( 0.856627153315983, 0.137318972929847, 0.11189821299995 ),
		vec3( 0.0951212405381588, 0.761241990602591, 0.0767994186031903 ),
		vec3( 0.0482516061458583, 0.101439036467562, 0.811302368396859 )
	);
	const mat3 AgXOutsetMatrix = mat3(
		vec3( 1.1271005818144368, - 0.1413297634984383, - 0.14132976349843826 ),
		vec3( - 0.11060664309660323, 1.157823702216272, - 0.11060664309660294 ),
		vec3( - 0.016493938717834573, - 0.016493938717834257, 1.2519364065950405 )
	);
	const float AgxMinEv = - 12.47393;	const float AgxMaxEv = 4.026069;
	color = LINEAR_SRGB_TO_LINEAR_REC2020 * color;
	color *= toneMappingExposure;
	color = AgXInsetMatrix * color;
	color = max( color, 1e-10 );	color = log2( color );
	color = ( color - AgxMinEv ) / ( AgxMaxEv - AgxMinEv );
	color = clamp( color, 0.0, 1.0 );
	color = agxDefaultContrastApprox( color );
	color = AgXOutsetMatrix * color;
	color = pow( max( vec3( 0.0 ), color ), vec3( 2.2 ) );
	color = LINEAR_REC2020_TO_LINEAR_SRGB * color;
	return color;
}
vec3 CustomToneMapping( vec3 color ) { return color; }`,G0=`#ifdef USE_TRANSMISSION
	material.transmission = transmission;
	material.transmissionAlpha = 1.0;
	material.thickness = thickness;
	material.attenuationDistance = attenuationDistance;
	material.attenuationColor = attenuationColor;
	#ifdef USE_TRANSMISSIONMAP
		material.transmission *= texture2D( transmissionMap, vTransmissionMapUv ).r;
	#endif
	#ifdef USE_THICKNESSMAP
		material.thickness *= texture2D( thicknessMap, vThicknessMapUv ).g;
	#endif
	vec3 pos = vWorldPosition;
	vec3 v = normalize( cameraPosition - pos );
	vec3 n = inverseTransformDirection( normal, viewMatrix );
	vec4 transmitted = getIBLVolumeRefraction(
		n, v, material.roughness, material.diffuseColor, material.specularColor, material.specularF90,
		pos, modelMatrix, viewMatrix, projectionMatrix, material.ior, material.thickness,
		material.attenuationColor, material.attenuationDistance );
	material.transmissionAlpha = mix( material.transmissionAlpha, transmitted.a, material.transmission );
	totalDiffuse = mix( totalDiffuse, transmitted.rgb, material.transmission );
#endif`,W0=`#ifdef USE_TRANSMISSION
	uniform float transmission;
	uniform float thickness;
	uniform float attenuationDistance;
	uniform vec3 attenuationColor;
	#ifdef USE_TRANSMISSIONMAP
		uniform sampler2D transmissionMap;
	#endif
	#ifdef USE_THICKNESSMAP
		uniform sampler2D thicknessMap;
	#endif
	uniform vec2 transmissionSamplerSize;
	uniform sampler2D transmissionSamplerMap;
	uniform mat4 modelMatrix;
	uniform mat4 projectionMatrix;
	varying vec3 vWorldPosition;
	float w0( float a ) {
		return ( 1.0 / 6.0 ) * ( a * ( a * ( - a + 3.0 ) - 3.0 ) + 1.0 );
	}
	float w1( float a ) {
		return ( 1.0 / 6.0 ) * ( a *  a * ( 3.0 * a - 6.0 ) + 4.0 );
	}
	float w2( float a ){
		return ( 1.0 / 6.0 ) * ( a * ( a * ( - 3.0 * a + 3.0 ) + 3.0 ) + 1.0 );
	}
	float w3( float a ) {
		return ( 1.0 / 6.0 ) * ( a * a * a );
	}
	float g0( float a ) {
		return w0( a ) + w1( a );
	}
	float g1( float a ) {
		return w2( a ) + w3( a );
	}
	float h0( float a ) {
		return - 1.0 + w1( a ) / ( w0( a ) + w1( a ) );
	}
	float h1( float a ) {
		return 1.0 + w3( a ) / ( w2( a ) + w3( a ) );
	}
	vec4 bicubic( sampler2D tex, vec2 uv, vec4 texelSize, float lod ) {
		uv = uv * texelSize.zw + 0.5;
		vec2 iuv = floor( uv );
		vec2 fuv = fract( uv );
		float g0x = g0( fuv.x );
		float g1x = g1( fuv.x );
		float h0x = h0( fuv.x );
		float h1x = h1( fuv.x );
		float h0y = h0( fuv.y );
		float h1y = h1( fuv.y );
		vec2 p0 = ( vec2( iuv.x + h0x, iuv.y + h0y ) - 0.5 ) * texelSize.xy;
		vec2 p1 = ( vec2( iuv.x + h1x, iuv.y + h0y ) - 0.5 ) * texelSize.xy;
		vec2 p2 = ( vec2( iuv.x + h0x, iuv.y + h1y ) - 0.5 ) * texelSize.xy;
		vec2 p3 = ( vec2( iuv.x + h1x, iuv.y + h1y ) - 0.5 ) * texelSize.xy;
		return g0( fuv.y ) * ( g0x * textureLod( tex, p0, lod ) + g1x * textureLod( tex, p1, lod ) ) +
			g1( fuv.y ) * ( g0x * textureLod( tex, p2, lod ) + g1x * textureLod( tex, p3, lod ) );
	}
	vec4 textureBicubic( sampler2D sampler, vec2 uv, float lod ) {
		vec2 fLodSize = vec2( textureSize( sampler, int( lod ) ) );
		vec2 cLodSize = vec2( textureSize( sampler, int( lod + 1.0 ) ) );
		vec2 fLodSizeInv = 1.0 / fLodSize;
		vec2 cLodSizeInv = 1.0 / cLodSize;
		vec4 fSample = bicubic( sampler, uv, vec4( fLodSizeInv, fLodSize ), floor( lod ) );
		vec4 cSample = bicubic( sampler, uv, vec4( cLodSizeInv, cLodSize ), ceil( lod ) );
		return mix( fSample, cSample, fract( lod ) );
	}
	vec3 getVolumeTransmissionRay( const in vec3 n, const in vec3 v, const in float thickness, const in float ior, const in mat4 modelMatrix ) {
		vec3 refractionVector = refract( - v, normalize( n ), 1.0 / ior );
		vec3 modelScale;
		modelScale.x = length( vec3( modelMatrix[ 0 ].xyz ) );
		modelScale.y = length( vec3( modelMatrix[ 1 ].xyz ) );
		modelScale.z = length( vec3( modelMatrix[ 2 ].xyz ) );
		return normalize( refractionVector ) * thickness * modelScale;
	}
	float applyIorToRoughness( const in float roughness, const in float ior ) {
		return roughness * clamp( ior * 2.0 - 2.0, 0.0, 1.0 );
	}
	vec4 getTransmissionSample( const in vec2 fragCoord, const in float roughness, const in float ior ) {
		float lod = log2( transmissionSamplerSize.x ) * applyIorToRoughness( roughness, ior );
		return textureBicubic( transmissionSamplerMap, fragCoord.xy, lod );
	}
	vec3 volumeAttenuation( const in float transmissionDistance, const in vec3 attenuationColor, const in float attenuationDistance ) {
		if ( isinf( attenuationDistance ) ) {
			return vec3( 1.0 );
		} else {
			vec3 attenuationCoefficient = -log( attenuationColor ) / attenuationDistance;
			vec3 transmittance = exp( - attenuationCoefficient * transmissionDistance );			return transmittance;
		}
	}
	vec4 getIBLVolumeRefraction( const in vec3 n, const in vec3 v, const in float roughness, const in vec3 diffuseColor,
		const in vec3 specularColor, const in float specularF90, const in vec3 position, const in mat4 modelMatrix,
		const in mat4 viewMatrix, const in mat4 projMatrix, const in float ior, const in float thickness,
		const in vec3 attenuationColor, const in float attenuationDistance ) {
		vec3 transmissionRay = getVolumeTransmissionRay( n, v, thickness, ior, modelMatrix );
		vec3 refractedRayExit = position + transmissionRay;
		vec4 ndcPos = projMatrix * viewMatrix * vec4( refractedRayExit, 1.0 );
		vec2 refractionCoords = ndcPos.xy / ndcPos.w;
		refractionCoords += 1.0;
		refractionCoords /= 2.0;
		vec4 transmittedLight = getTransmissionSample( refractionCoords, roughness, ior );
		vec3 transmittance = diffuseColor * volumeAttenuation( length( transmissionRay ), attenuationColor, attenuationDistance );
		vec3 attenuatedColor = transmittance * transmittedLight.rgb;
		vec3 F = EnvironmentBRDF( n, v, specularColor, specularF90, roughness );
		float transmittanceFactor = ( transmittance.r + transmittance.g + transmittance.b ) / 3.0;
		return vec4( ( 1.0 - F ) * attenuatedColor, 1.0 - ( 1.0 - transmittedLight.a ) * transmittanceFactor );
	}
#endif`,q0=`#if defined( USE_UV ) || defined( USE_ANISOTROPY )
	varying vec2 vUv;
#endif
#ifdef USE_MAP
	varying vec2 vMapUv;
#endif
#ifdef USE_ALPHAMAP
	varying vec2 vAlphaMapUv;
#endif
#ifdef USE_LIGHTMAP
	varying vec2 vLightMapUv;
#endif
#ifdef USE_AOMAP
	varying vec2 vAoMapUv;
#endif
#ifdef USE_BUMPMAP
	varying vec2 vBumpMapUv;
#endif
#ifdef USE_NORMALMAP
	varying vec2 vNormalMapUv;
#endif
#ifdef USE_EMISSIVEMAP
	varying vec2 vEmissiveMapUv;
#endif
#ifdef USE_METALNESSMAP
	varying vec2 vMetalnessMapUv;
#endif
#ifdef USE_ROUGHNESSMAP
	varying vec2 vRoughnessMapUv;
#endif
#ifdef USE_ANISOTROPYMAP
	varying vec2 vAnisotropyMapUv;
#endif
#ifdef USE_CLEARCOATMAP
	varying vec2 vClearcoatMapUv;
#endif
#ifdef USE_CLEARCOAT_NORMALMAP
	varying vec2 vClearcoatNormalMapUv;
#endif
#ifdef USE_CLEARCOAT_ROUGHNESSMAP
	varying vec2 vClearcoatRoughnessMapUv;
#endif
#ifdef USE_IRIDESCENCEMAP
	varying vec2 vIridescenceMapUv;
#endif
#ifdef USE_IRIDESCENCE_THICKNESSMAP
	varying vec2 vIridescenceThicknessMapUv;
#endif
#ifdef USE_SHEEN_COLORMAP
	varying vec2 vSheenColorMapUv;
#endif
#ifdef USE_SHEEN_ROUGHNESSMAP
	varying vec2 vSheenRoughnessMapUv;
#endif
#ifdef USE_SPECULARMAP
	varying vec2 vSpecularMapUv;
#endif
#ifdef USE_SPECULAR_COLORMAP
	varying vec2 vSpecularColorMapUv;
#endif
#ifdef USE_SPECULAR_INTENSITYMAP
	varying vec2 vSpecularIntensityMapUv;
#endif
#ifdef USE_TRANSMISSIONMAP
	uniform mat3 transmissionMapTransform;
	varying vec2 vTransmissionMapUv;
#endif
#ifdef USE_THICKNESSMAP
	uniform mat3 thicknessMapTransform;
	varying vec2 vThicknessMapUv;
#endif`,X0=`#if defined( USE_UV ) || defined( USE_ANISOTROPY )
	varying vec2 vUv;
#endif
#ifdef USE_MAP
	uniform mat3 mapTransform;
	varying vec2 vMapUv;
#endif
#ifdef USE_ALPHAMAP
	uniform mat3 alphaMapTransform;
	varying vec2 vAlphaMapUv;
#endif
#ifdef USE_LIGHTMAP
	uniform mat3 lightMapTransform;
	varying vec2 vLightMapUv;
#endif
#ifdef USE_AOMAP
	uniform mat3 aoMapTransform;
	varying vec2 vAoMapUv;
#endif
#ifdef USE_BUMPMAP
	uniform mat3 bumpMapTransform;
	varying vec2 vBumpMapUv;
#endif
#ifdef USE_NORMALMAP
	uniform mat3 normalMapTransform;
	varying vec2 vNormalMapUv;
#endif
#ifdef USE_DISPLACEMENTMAP
	uniform mat3 displacementMapTransform;
	varying vec2 vDisplacementMapUv;
#endif
#ifdef USE_EMISSIVEMAP
	uniform mat3 emissiveMapTransform;
	varying vec2 vEmissiveMapUv;
#endif
#ifdef USE_METALNESSMAP
	uniform mat3 metalnessMapTransform;
	varying vec2 vMetalnessMapUv;
#endif
#ifdef USE_ROUGHNESSMAP
	uniform mat3 roughnessMapTransform;
	varying vec2 vRoughnessMapUv;
#endif
#ifdef USE_ANISOTROPYMAP
	uniform mat3 anisotropyMapTransform;
	varying vec2 vAnisotropyMapUv;
#endif
#ifdef USE_CLEARCOATMAP
	uniform mat3 clearcoatMapTransform;
	varying vec2 vClearcoatMapUv;
#endif
#ifdef USE_CLEARCOAT_NORMALMAP
	uniform mat3 clearcoatNormalMapTransform;
	varying vec2 vClearcoatNormalMapUv;
#endif
#ifdef USE_CLEARCOAT_ROUGHNESSMAP
	uniform mat3 clearcoatRoughnessMapTransform;
	varying vec2 vClearcoatRoughnessMapUv;
#endif
#ifdef USE_SHEEN_COLORMAP
	uniform mat3 sheenColorMapTransform;
	varying vec2 vSheenColorMapUv;
#endif
#ifdef USE_SHEEN_ROUGHNESSMAP
	uniform mat3 sheenRoughnessMapTransform;
	varying vec2 vSheenRoughnessMapUv;
#endif
#ifdef USE_IRIDESCENCEMAP
	uniform mat3 iridescenceMapTransform;
	varying vec2 vIridescenceMapUv;
#endif
#ifdef USE_IRIDESCENCE_THICKNESSMAP
	uniform mat3 iridescenceThicknessMapTransform;
	varying vec2 vIridescenceThicknessMapUv;
#endif
#ifdef USE_SPECULARMAP
	uniform mat3 specularMapTransform;
	varying vec2 vSpecularMapUv;
#endif
#ifdef USE_SPECULAR_COLORMAP
	uniform mat3 specularColorMapTransform;
	varying vec2 vSpecularColorMapUv;
#endif
#ifdef USE_SPECULAR_INTENSITYMAP
	uniform mat3 specularIntensityMapTransform;
	varying vec2 vSpecularIntensityMapUv;
#endif
#ifdef USE_TRANSMISSIONMAP
	uniform mat3 transmissionMapTransform;
	varying vec2 vTransmissionMapUv;
#endif
#ifdef USE_THICKNESSMAP
	uniform mat3 thicknessMapTransform;
	varying vec2 vThicknessMapUv;
#endif`,j0=`#if defined( USE_UV ) || defined( USE_ANISOTROPY )
	vUv = vec3( uv, 1 ).xy;
#endif
#ifdef USE_MAP
	vMapUv = ( mapTransform * vec3( MAP_UV, 1 ) ).xy;
#endif
#ifdef USE_ALPHAMAP
	vAlphaMapUv = ( alphaMapTransform * vec3( ALPHAMAP_UV, 1 ) ).xy;
#endif
#ifdef USE_LIGHTMAP
	vLightMapUv = ( lightMapTransform * vec3( LIGHTMAP_UV, 1 ) ).xy;
#endif
#ifdef USE_AOMAP
	vAoMapUv = ( aoMapTransform * vec3( AOMAP_UV, 1 ) ).xy;
#endif
#ifdef USE_BUMPMAP
	vBumpMapUv = ( bumpMapTransform * vec3( BUMPMAP_UV, 1 ) ).xy;
#endif
#ifdef USE_NORMALMAP
	vNormalMapUv = ( normalMapTransform * vec3( NORMALMAP_UV, 1 ) ).xy;
#endif
#ifdef USE_DISPLACEMENTMAP
	vDisplacementMapUv = ( displacementMapTransform * vec3( DISPLACEMENTMAP_UV, 1 ) ).xy;
#endif
#ifdef USE_EMISSIVEMAP
	vEmissiveMapUv = ( emissiveMapTransform * vec3( EMISSIVEMAP_UV, 1 ) ).xy;
#endif
#ifdef USE_METALNESSMAP
	vMetalnessMapUv = ( metalnessMapTransform * vec3( METALNESSMAP_UV, 1 ) ).xy;
#endif
#ifdef USE_ROUGHNESSMAP
	vRoughnessMapUv = ( roughnessMapTransform * vec3( ROUGHNESSMAP_UV, 1 ) ).xy;
#endif
#ifdef USE_ANISOTROPYMAP
	vAnisotropyMapUv = ( anisotropyMapTransform * vec3( ANISOTROPYMAP_UV, 1 ) ).xy;
#endif
#ifdef USE_CLEARCOATMAP
	vClearcoatMapUv = ( clearcoatMapTransform * vec3( CLEARCOATMAP_UV, 1 ) ).xy;
#endif
#ifdef USE_CLEARCOAT_NORMALMAP
	vClearcoatNormalMapUv = ( clearcoatNormalMapTransform * vec3( CLEARCOAT_NORMALMAP_UV, 1 ) ).xy;
#endif
#ifdef USE_CLEARCOAT_ROUGHNESSMAP
	vClearcoatRoughnessMapUv = ( clearcoatRoughnessMapTransform * vec3( CLEARCOAT_ROUGHNESSMAP_UV, 1 ) ).xy;
#endif
#ifdef USE_IRIDESCENCEMAP
	vIridescenceMapUv = ( iridescenceMapTransform * vec3( IRIDESCENCEMAP_UV, 1 ) ).xy;
#endif
#ifdef USE_IRIDESCENCE_THICKNESSMAP
	vIridescenceThicknessMapUv = ( iridescenceThicknessMapTransform * vec3( IRIDESCENCE_THICKNESSMAP_UV, 1 ) ).xy;
#endif
#ifdef USE_SHEEN_COLORMAP
	vSheenColorMapUv = ( sheenColorMapTransform * vec3( SHEEN_COLORMAP_UV, 1 ) ).xy;
#endif
#ifdef USE_SHEEN_ROUGHNESSMAP
	vSheenRoughnessMapUv = ( sheenRoughnessMapTransform * vec3( SHEEN_ROUGHNESSMAP_UV, 1 ) ).xy;
#endif
#ifdef USE_SPECULARMAP
	vSpecularMapUv = ( specularMapTransform * vec3( SPECULARMAP_UV, 1 ) ).xy;
#endif
#ifdef USE_SPECULAR_COLORMAP
	vSpecularColorMapUv = ( specularColorMapTransform * vec3( SPECULAR_COLORMAP_UV, 1 ) ).xy;
#endif
#ifdef USE_SPECULAR_INTENSITYMAP
	vSpecularIntensityMapUv = ( specularIntensityMapTransform * vec3( SPECULAR_INTENSITYMAP_UV, 1 ) ).xy;
#endif
#ifdef USE_TRANSMISSIONMAP
	vTransmissionMapUv = ( transmissionMapTransform * vec3( TRANSMISSIONMAP_UV, 1 ) ).xy;
#endif
#ifdef USE_THICKNESSMAP
	vThicknessMapUv = ( thicknessMapTransform * vec3( THICKNESSMAP_UV, 1 ) ).xy;
#endif`,Y0=`#if defined( USE_ENVMAP ) || defined( DISTANCE ) || defined ( USE_SHADOWMAP ) || defined ( USE_TRANSMISSION ) || NUM_SPOT_LIGHT_COORDS > 0
	vec4 worldPosition = vec4( transformed, 1.0 );
	#ifdef USE_BATCHING
		worldPosition = batchingMatrix * worldPosition;
	#endif
	#ifdef USE_INSTANCING
		worldPosition = instanceMatrix * worldPosition;
	#endif
	worldPosition = modelMatrix * worldPosition;
#endif`;const $0=`varying vec2 vUv;
uniform mat3 uvTransform;
void main() {
	vUv = ( uvTransform * vec3( uv, 1 ) ).xy;
	gl_Position = vec4( position.xy, 1.0, 1.0 );
}`,K0=`uniform sampler2D t2D;
uniform float backgroundIntensity;
varying vec2 vUv;
void main() {
	vec4 texColor = texture2D( t2D, vUv );
	#ifdef DECODE_VIDEO_TEXTURE
		texColor = vec4( mix( pow( texColor.rgb * 0.9478672986 + vec3( 0.0521327014 ), vec3( 2.4 ) ), texColor.rgb * 0.0773993808, vec3( lessThanEqual( texColor.rgb, vec3( 0.04045 ) ) ) ), texColor.w );
	#endif
	texColor.rgb *= backgroundIntensity;
	gl_FragColor = texColor;
	#include <tonemapping_fragment>
	#include <colorspace_fragment>
}`,Q0=`varying vec3 vWorldDirection;
#include <common>
void main() {
	vWorldDirection = transformDirection( position, modelMatrix );
	#include <begin_vertex>
	#include <project_vertex>
	gl_Position.z = gl_Position.w;
}`,Z0=`#ifdef ENVMAP_TYPE_CUBE
	uniform samplerCube envMap;
#elif defined( ENVMAP_TYPE_CUBE_UV )
	uniform sampler2D envMap;
#endif
uniform float flipEnvMap;
uniform float backgroundBlurriness;
uniform float backgroundIntensity;
varying vec3 vWorldDirection;
#include <cube_uv_reflection_fragment>
void main() {
	#ifdef ENVMAP_TYPE_CUBE
		vec4 texColor = textureCube( envMap, vec3( flipEnvMap * vWorldDirection.x, vWorldDirection.yz ) );
	#elif defined( ENVMAP_TYPE_CUBE_UV )
		vec4 texColor = textureCubeUV( envMap, vWorldDirection, backgroundBlurriness );
	#else
		vec4 texColor = vec4( 0.0, 0.0, 0.0, 1.0 );
	#endif
	texColor.rgb *= backgroundIntensity;
	gl_FragColor = texColor;
	#include <tonemapping_fragment>
	#include <colorspace_fragment>
}`,J0=`varying vec3 vWorldDirection;
#include <common>
void main() {
	vWorldDirection = transformDirection( position, modelMatrix );
	#include <begin_vertex>
	#include <project_vertex>
	gl_Position.z = gl_Position.w;
}`,ty=`uniform samplerCube tCube;
uniform float tFlip;
uniform float opacity;
varying vec3 vWorldDirection;
void main() {
	vec4 texColor = textureCube( tCube, vec3( tFlip * vWorldDirection.x, vWorldDirection.yz ) );
	gl_FragColor = texColor;
	gl_FragColor.a *= opacity;
	#include <tonemapping_fragment>
	#include <colorspace_fragment>
}`,ey=`#include <common>
#include <batching_pars_vertex>
#include <uv_pars_vertex>
#include <displacementmap_pars_vertex>
#include <morphtarget_pars_vertex>
#include <skinning_pars_vertex>
#include <logdepthbuf_pars_vertex>
#include <clipping_planes_pars_vertex>
varying vec2 vHighPrecisionZW;
void main() {
	#include <uv_vertex>
	#include <batching_vertex>
	#include <skinbase_vertex>
	#ifdef USE_DISPLACEMENTMAP
		#include <beginnormal_vertex>
		#include <morphnormal_vertex>
		#include <skinnormal_vertex>
	#endif
	#include <begin_vertex>
	#include <morphtarget_vertex>
	#include <skinning_vertex>
	#include <displacementmap_vertex>
	#include <project_vertex>
	#include <logdepthbuf_vertex>
	#include <clipping_planes_vertex>
	vHighPrecisionZW = gl_Position.zw;
}`,ny=`#if DEPTH_PACKING == 3200
	uniform float opacity;
#endif
#include <common>
#include <packing>
#include <uv_pars_fragment>
#include <map_pars_fragment>
#include <alphamap_pars_fragment>
#include <alphatest_pars_fragment>
#include <alphahash_pars_fragment>
#include <logdepthbuf_pars_fragment>
#include <clipping_planes_pars_fragment>
varying vec2 vHighPrecisionZW;
void main() {
	#include <clipping_planes_fragment>
	vec4 diffuseColor = vec4( 1.0 );
	#if DEPTH_PACKING == 3200
		diffuseColor.a = opacity;
	#endif
	#include <map_fragment>
	#include <alphamap_fragment>
	#include <alphatest_fragment>
	#include <alphahash_fragment>
	#include <logdepthbuf_fragment>
	float fragCoordZ = 0.5 * vHighPrecisionZW[0] / vHighPrecisionZW[1] + 0.5;
	#if DEPTH_PACKING == 3200
		gl_FragColor = vec4( vec3( 1.0 - fragCoordZ ), opacity );
	#elif DEPTH_PACKING == 3201
		gl_FragColor = packDepthToRGBA( fragCoordZ );
	#endif
}`,iy=`#define DISTANCE
varying vec3 vWorldPosition;
#include <common>
#include <batching_pars_vertex>
#include <uv_pars_vertex>
#include <displacementmap_pars_vertex>
#include <morphtarget_pars_vertex>
#include <skinning_pars_vertex>
#include <clipping_planes_pars_vertex>
void main() {
	#include <uv_vertex>
	#include <batching_vertex>
	#include <skinbase_vertex>
	#ifdef USE_DISPLACEMENTMAP
		#include <beginnormal_vertex>
		#include <morphnormal_vertex>
		#include <skinnormal_vertex>
	#endif
	#include <begin_vertex>
	#include <morphtarget_vertex>
	#include <skinning_vertex>
	#include <displacementmap_vertex>
	#include <project_vertex>
	#include <worldpos_vertex>
	#include <clipping_planes_vertex>
	vWorldPosition = worldPosition.xyz;
}`,ry=`#define DISTANCE
uniform vec3 referencePosition;
uniform float nearDistance;
uniform float farDistance;
varying vec3 vWorldPosition;
#include <common>
#include <packing>
#include <uv_pars_fragment>
#include <map_pars_fragment>
#include <alphamap_pars_fragment>
#include <alphatest_pars_fragment>
#include <alphahash_pars_fragment>
#include <clipping_planes_pars_fragment>
void main () {
	#include <clipping_planes_fragment>
	vec4 diffuseColor = vec4( 1.0 );
	#include <map_fragment>
	#include <alphamap_fragment>
	#include <alphatest_fragment>
	#include <alphahash_fragment>
	float dist = length( vWorldPosition - referencePosition );
	dist = ( dist - nearDistance ) / ( farDistance - nearDistance );
	dist = saturate( dist );
	gl_FragColor = packDepthToRGBA( dist );
}`,sy=`varying vec3 vWorldDirection;
#include <common>
void main() {
	vWorldDirection = transformDirection( position, modelMatrix );
	#include <begin_vertex>
	#include <project_vertex>
}`,oy=`uniform sampler2D tEquirect;
varying vec3 vWorldDirection;
#include <common>
void main() {
	vec3 direction = normalize( vWorldDirection );
	vec2 sampleUV = equirectUv( direction );
	gl_FragColor = texture2D( tEquirect, sampleUV );
	#include <tonemapping_fragment>
	#include <colorspace_fragment>
}`,ay=`uniform float scale;
attribute float lineDistance;
varying float vLineDistance;
#include <common>
#include <uv_pars_vertex>
#include <color_pars_vertex>
#include <fog_pars_vertex>
#include <morphtarget_pars_vertex>
#include <logdepthbuf_pars_vertex>
#include <clipping_planes_pars_vertex>
void main() {
	vLineDistance = scale * lineDistance;
	#include <uv_vertex>
	#include <color_vertex>
	#include <morphcolor_vertex>
	#include <begin_vertex>
	#include <morphtarget_vertex>
	#include <project_vertex>
	#include <logdepthbuf_vertex>
	#include <clipping_planes_vertex>
	#include <fog_vertex>
}`,cy=`uniform vec3 diffuse;
uniform float opacity;
uniform float dashSize;
uniform float totalSize;
varying float vLineDistance;
#include <common>
#include <color_pars_fragment>
#include <uv_pars_fragment>
#include <map_pars_fragment>
#include <fog_pars_fragment>
#include <logdepthbuf_pars_fragment>
#include <clipping_planes_pars_fragment>
void main() {
	#include <clipping_planes_fragment>
	if ( mod( vLineDistance, totalSize ) > dashSize ) {
		discard;
	}
	vec3 outgoingLight = vec3( 0.0 );
	vec4 diffuseColor = vec4( diffuse, opacity );
	#include <logdepthbuf_fragment>
	#include <map_fragment>
	#include <color_fragment>
	outgoingLight = diffuseColor.rgb;
	#include <opaque_fragment>
	#include <tonemapping_fragment>
	#include <colorspace_fragment>
	#include <fog_fragment>
	#include <premultiplied_alpha_fragment>
}`,ly=`#include <common>
#include <batching_pars_vertex>
#include <uv_pars_vertex>
#include <envmap_pars_vertex>
#include <color_pars_vertex>
#include <fog_pars_vertex>
#include <morphtarget_pars_vertex>
#include <skinning_pars_vertex>
#include <logdepthbuf_pars_vertex>
#include <clipping_planes_pars_vertex>
void main() {
	#include <uv_vertex>
	#include <color_vertex>
	#include <morphcolor_vertex>
	#include <batching_vertex>
	#if defined ( USE_ENVMAP ) || defined ( USE_SKINNING )
		#include <beginnormal_vertex>
		#include <morphnormal_vertex>
		#include <skinbase_vertex>
		#include <skinnormal_vertex>
		#include <defaultnormal_vertex>
	#endif
	#include <begin_vertex>
	#include <morphtarget_vertex>
	#include <skinning_vertex>
	#include <project_vertex>
	#include <logdepthbuf_vertex>
	#include <clipping_planes_vertex>
	#include <worldpos_vertex>
	#include <envmap_vertex>
	#include <fog_vertex>
}`,hy=`uniform vec3 diffuse;
uniform float opacity;
#ifndef FLAT_SHADED
	varying vec3 vNormal;
#endif
#include <common>
#include <dithering_pars_fragment>
#include <color_pars_fragment>
#include <uv_pars_fragment>
#include <map_pars_fragment>
#include <alphamap_pars_fragment>
#include <alphatest_pars_fragment>
#include <alphahash_pars_fragment>
#include <aomap_pars_fragment>
#include <lightmap_pars_fragment>
#include <envmap_common_pars_fragment>
#include <envmap_pars_fragment>
#include <fog_pars_fragment>
#include <specularmap_pars_fragment>
#include <logdepthbuf_pars_fragment>
#include <clipping_planes_pars_fragment>
void main() {
	#include <clipping_planes_fragment>
	vec4 diffuseColor = vec4( diffuse, opacity );
	#include <logdepthbuf_fragment>
	#include <map_fragment>
	#include <color_fragment>
	#include <alphamap_fragment>
	#include <alphatest_fragment>
	#include <alphahash_fragment>
	#include <specularmap_fragment>
	ReflectedLight reflectedLight = ReflectedLight( vec3( 0.0 ), vec3( 0.0 ), vec3( 0.0 ), vec3( 0.0 ) );
	#ifdef USE_LIGHTMAP
		vec4 lightMapTexel = texture2D( lightMap, vLightMapUv );
		reflectedLight.indirectDiffuse += lightMapTexel.rgb * lightMapIntensity * RECIPROCAL_PI;
	#else
		reflectedLight.indirectDiffuse += vec3( 1.0 );
	#endif
	#include <aomap_fragment>
	reflectedLight.indirectDiffuse *= diffuseColor.rgb;
	vec3 outgoingLight = reflectedLight.indirectDiffuse;
	#include <envmap_fragment>
	#include <opaque_fragment>
	#include <tonemapping_fragment>
	#include <colorspace_fragment>
	#include <fog_fragment>
	#include <premultiplied_alpha_fragment>
	#include <dithering_fragment>
}`,uy=`#define LAMBERT
varying vec3 vViewPosition;
#include <common>
#include <batching_pars_vertex>
#include <uv_pars_vertex>
#include <displacementmap_pars_vertex>
#include <envmap_pars_vertex>
#include <color_pars_vertex>
#include <fog_pars_vertex>
#include <normal_pars_vertex>
#include <morphtarget_pars_vertex>
#include <skinning_pars_vertex>
#include <shadowmap_pars_vertex>
#include <logdepthbuf_pars_vertex>
#include <clipping_planes_pars_vertex>
void main() {
	#include <uv_vertex>
	#include <color_vertex>
	#include <morphcolor_vertex>
	#include <batching_vertex>
	#include <beginnormal_vertex>
	#include <morphnormal_vertex>
	#include <skinbase_vertex>
	#include <skinnormal_vertex>
	#include <defaultnormal_vertex>
	#include <normal_vertex>
	#include <begin_vertex>
	#include <morphtarget_vertex>
	#include <skinning_vertex>
	#include <displacementmap_vertex>
	#include <project_vertex>
	#include <logdepthbuf_vertex>
	#include <clipping_planes_vertex>
	vViewPosition = - mvPosition.xyz;
	#include <worldpos_vertex>
	#include <envmap_vertex>
	#include <shadowmap_vertex>
	#include <fog_vertex>
}`,dy=`#define LAMBERT
uniform vec3 diffuse;
uniform vec3 emissive;
uniform float opacity;
#include <common>
#include <packing>
#include <dithering_pars_fragment>
#include <color_pars_fragment>
#include <uv_pars_fragment>
#include <map_pars_fragment>
#include <alphamap_pars_fragment>
#include <alphatest_pars_fragment>
#include <alphahash_pars_fragment>
#include <aomap_pars_fragment>
#include <lightmap_pars_fragment>
#include <emissivemap_pars_fragment>
#include <envmap_common_pars_fragment>
#include <envmap_pars_fragment>
#include <fog_pars_fragment>
#include <bsdfs>
#include <lights_pars_begin>
#include <normal_pars_fragment>
#include <lights_lambert_pars_fragment>
#include <shadowmap_pars_fragment>
#include <bumpmap_pars_fragment>
#include <normalmap_pars_fragment>
#include <specularmap_pars_fragment>
#include <logdepthbuf_pars_fragment>
#include <clipping_planes_pars_fragment>
void main() {
	#include <clipping_planes_fragment>
	vec4 diffuseColor = vec4( diffuse, opacity );
	ReflectedLight reflectedLight = ReflectedLight( vec3( 0.0 ), vec3( 0.0 ), vec3( 0.0 ), vec3( 0.0 ) );
	vec3 totalEmissiveRadiance = emissive;
	#include <logdepthbuf_fragment>
	#include <map_fragment>
	#include <color_fragment>
	#include <alphamap_fragment>
	#include <alphatest_fragment>
	#include <alphahash_fragment>
	#include <specularmap_fragment>
	#include <normal_fragment_begin>
	#include <normal_fragment_maps>
	#include <emissivemap_fragment>
	#include <lights_lambert_fragment>
	#include <lights_fragment_begin>
	#include <lights_fragment_maps>
	#include <lights_fragment_end>
	#include <aomap_fragment>
	vec3 outgoingLight = reflectedLight.directDiffuse + reflectedLight.indirectDiffuse + totalEmissiveRadiance;
	#include <envmap_fragment>
	#include <opaque_fragment>
	#include <tonemapping_fragment>
	#include <colorspace_fragment>
	#include <fog_fragment>
	#include <premultiplied_alpha_fragment>
	#include <dithering_fragment>
}`,fy=`#define MATCAP
varying vec3 vViewPosition;
#include <common>
#include <batching_pars_vertex>
#include <uv_pars_vertex>
#include <color_pars_vertex>
#include <displacementmap_pars_vertex>
#include <fog_pars_vertex>
#include <normal_pars_vertex>
#include <morphtarget_pars_vertex>
#include <skinning_pars_vertex>
#include <logdepthbuf_pars_vertex>
#include <clipping_planes_pars_vertex>
void main() {
	#include <uv_vertex>
	#include <color_vertex>
	#include <morphcolor_vertex>
	#include <batching_vertex>
	#include <beginnormal_vertex>
	#include <morphnormal_vertex>
	#include <skinbase_vertex>
	#include <skinnormal_vertex>
	#include <defaultnormal_vertex>
	#include <normal_vertex>
	#include <begin_vertex>
	#include <morphtarget_vertex>
	#include <skinning_vertex>
	#include <displacementmap_vertex>
	#include <project_vertex>
	#include <logdepthbuf_vertex>
	#include <clipping_planes_vertex>
	#include <fog_vertex>
	vViewPosition = - mvPosition.xyz;
}`,py=`#define MATCAP
uniform vec3 diffuse;
uniform float opacity;
uniform sampler2D matcap;
varying vec3 vViewPosition;
#include <common>
#include <dithering_pars_fragment>
#include <color_pars_fragment>
#include <uv_pars_fragment>
#include <map_pars_fragment>
#include <alphamap_pars_fragment>
#include <alphatest_pars_fragment>
#include <alphahash_pars_fragment>
#include <fog_pars_fragment>
#include <normal_pars_fragment>
#include <bumpmap_pars_fragment>
#include <normalmap_pars_fragment>
#include <logdepthbuf_pars_fragment>
#include <clipping_planes_pars_fragment>
void main() {
	#include <clipping_planes_fragment>
	vec4 diffuseColor = vec4( diffuse, opacity );
	#include <logdepthbuf_fragment>
	#include <map_fragment>
	#include <color_fragment>
	#include <alphamap_fragment>
	#include <alphatest_fragment>
	#include <alphahash_fragment>
	#include <normal_fragment_begin>
	#include <normal_fragment_maps>
	vec3 viewDir = normalize( vViewPosition );
	vec3 x = normalize( vec3( viewDir.z, 0.0, - viewDir.x ) );
	vec3 y = cross( viewDir, x );
	vec2 uv = vec2( dot( x, normal ), dot( y, normal ) ) * 0.495 + 0.5;
	#ifdef USE_MATCAP
		vec4 matcapColor = texture2D( matcap, uv );
	#else
		vec4 matcapColor = vec4( vec3( mix( 0.2, 0.8, uv.y ) ), 1.0 );
	#endif
	vec3 outgoingLight = diffuseColor.rgb * matcapColor.rgb;
	#include <opaque_fragment>
	#include <tonemapping_fragment>
	#include <colorspace_fragment>
	#include <fog_fragment>
	#include <premultiplied_alpha_fragment>
	#include <dithering_fragment>
}`,my=`#define NORMAL
#if defined( FLAT_SHADED ) || defined( USE_BUMPMAP ) || defined( USE_NORMALMAP_TANGENTSPACE )
	varying vec3 vViewPosition;
#endif
#include <common>
#include <batching_pars_vertex>
#include <uv_pars_vertex>
#include <displacementmap_pars_vertex>
#include <normal_pars_vertex>
#include <morphtarget_pars_vertex>
#include <skinning_pars_vertex>
#include <logdepthbuf_pars_vertex>
#include <clipping_planes_pars_vertex>
void main() {
	#include <uv_vertex>
	#include <batching_vertex>
	#include <beginnormal_vertex>
	#include <morphnormal_vertex>
	#include <skinbase_vertex>
	#include <skinnormal_vertex>
	#include <defaultnormal_vertex>
	#include <normal_vertex>
	#include <begin_vertex>
	#include <morphtarget_vertex>
	#include <skinning_vertex>
	#include <displacementmap_vertex>
	#include <project_vertex>
	#include <logdepthbuf_vertex>
	#include <clipping_planes_vertex>
#if defined( FLAT_SHADED ) || defined( USE_BUMPMAP ) || defined( USE_NORMALMAP_TANGENTSPACE )
	vViewPosition = - mvPosition.xyz;
#endif
}`,gy=`#define NORMAL
uniform float opacity;
#if defined( FLAT_SHADED ) || defined( USE_BUMPMAP ) || defined( USE_NORMALMAP_TANGENTSPACE )
	varying vec3 vViewPosition;
#endif
#include <packing>
#include <uv_pars_fragment>
#include <normal_pars_fragment>
#include <bumpmap_pars_fragment>
#include <normalmap_pars_fragment>
#include <logdepthbuf_pars_fragment>
#include <clipping_planes_pars_fragment>
void main() {
	#include <clipping_planes_fragment>
	#include <logdepthbuf_fragment>
	#include <normal_fragment_begin>
	#include <normal_fragment_maps>
	gl_FragColor = vec4( packNormalToRGB( normal ), opacity );
	#ifdef OPAQUE
		gl_FragColor.a = 1.0;
	#endif
}`,_y=`#define PHONG
varying vec3 vViewPosition;
#include <common>
#include <batching_pars_vertex>
#include <uv_pars_vertex>
#include <displacementmap_pars_vertex>
#include <envmap_pars_vertex>
#include <color_pars_vertex>
#include <fog_pars_vertex>
#include <normal_pars_vertex>
#include <morphtarget_pars_vertex>
#include <skinning_pars_vertex>
#include <shadowmap_pars_vertex>
#include <logdepthbuf_pars_vertex>
#include <clipping_planes_pars_vertex>
void main() {
	#include <uv_vertex>
	#include <color_vertex>
	#include <morphcolor_vertex>
	#include <batching_vertex>
	#include <beginnormal_vertex>
	#include <morphnormal_vertex>
	#include <skinbase_vertex>
	#include <skinnormal_vertex>
	#include <defaultnormal_vertex>
	#include <normal_vertex>
	#include <begin_vertex>
	#include <morphtarget_vertex>
	#include <skinning_vertex>
	#include <displacementmap_vertex>
	#include <project_vertex>
	#include <logdepthbuf_vertex>
	#include <clipping_planes_vertex>
	vViewPosition = - mvPosition.xyz;
	#include <worldpos_vertex>
	#include <envmap_vertex>
	#include <shadowmap_vertex>
	#include <fog_vertex>
}`,vy=`#define PHONG
uniform vec3 diffuse;
uniform vec3 emissive;
uniform vec3 specular;
uniform float shininess;
uniform float opacity;
#include <common>
#include <packing>
#include <dithering_pars_fragment>
#include <color_pars_fragment>
#include <uv_pars_fragment>
#include <map_pars_fragment>
#include <alphamap_pars_fragment>
#include <alphatest_pars_fragment>
#include <alphahash_pars_fragment>
#include <aomap_pars_fragment>
#include <lightmap_pars_fragment>
#include <emissivemap_pars_fragment>
#include <envmap_common_pars_fragment>
#include <envmap_pars_fragment>
#include <fog_pars_fragment>
#include <bsdfs>
#include <lights_pars_begin>
#include <normal_pars_fragment>
#include <lights_phong_pars_fragment>
#include <shadowmap_pars_fragment>
#include <bumpmap_pars_fragment>
#include <normalmap_pars_fragment>
#include <specularmap_pars_fragment>
#include <logdepthbuf_pars_fragment>
#include <clipping_planes_pars_fragment>
void main() {
	#include <clipping_planes_fragment>
	vec4 diffuseColor = vec4( diffuse, opacity );
	ReflectedLight reflectedLight = ReflectedLight( vec3( 0.0 ), vec3( 0.0 ), vec3( 0.0 ), vec3( 0.0 ) );
	vec3 totalEmissiveRadiance = emissive;
	#include <logdepthbuf_fragment>
	#include <map_fragment>
	#include <color_fragment>
	#include <alphamap_fragment>
	#include <alphatest_fragment>
	#include <alphahash_fragment>
	#include <specularmap_fragment>
	#include <normal_fragment_begin>
	#include <normal_fragment_maps>
	#include <emissivemap_fragment>
	#include <lights_phong_fragment>
	#include <lights_fragment_begin>
	#include <lights_fragment_maps>
	#include <lights_fragment_end>
	#include <aomap_fragment>
	vec3 outgoingLight = reflectedLight.directDiffuse + reflectedLight.indirectDiffuse + reflectedLight.directSpecular + reflectedLight.indirectSpecular + totalEmissiveRadiance;
	#include <envmap_fragment>
	#include <opaque_fragment>
	#include <tonemapping_fragment>
	#include <colorspace_fragment>
	#include <fog_fragment>
	#include <premultiplied_alpha_fragment>
	#include <dithering_fragment>
}`,yy=`#define STANDARD
varying vec3 vViewPosition;
#ifdef USE_TRANSMISSION
	varying vec3 vWorldPosition;
#endif
#include <common>
#include <batching_pars_vertex>
#include <uv_pars_vertex>
#include <displacementmap_pars_vertex>
#include <color_pars_vertex>
#include <fog_pars_vertex>
#include <normal_pars_vertex>
#include <morphtarget_pars_vertex>
#include <skinning_pars_vertex>
#include <shadowmap_pars_vertex>
#include <logdepthbuf_pars_vertex>
#include <clipping_planes_pars_vertex>
void main() {
	#include <uv_vertex>
	#include <color_vertex>
	#include <morphcolor_vertex>
	#include <batching_vertex>
	#include <beginnormal_vertex>
	#include <morphnormal_vertex>
	#include <skinbase_vertex>
	#include <skinnormal_vertex>
	#include <defaultnormal_vertex>
	#include <normal_vertex>
	#include <begin_vertex>
	#include <morphtarget_vertex>
	#include <skinning_vertex>
	#include <displacementmap_vertex>
	#include <project_vertex>
	#include <logdepthbuf_vertex>
	#include <clipping_planes_vertex>
	vViewPosition = - mvPosition.xyz;
	#include <worldpos_vertex>
	#include <shadowmap_vertex>
	#include <fog_vertex>
#ifdef USE_TRANSMISSION
	vWorldPosition = worldPosition.xyz;
#endif
}`,Ey=`#define STANDARD
#ifdef PHYSICAL
	#define IOR
	#define USE_SPECULAR
#endif
uniform vec3 diffuse;
uniform vec3 emissive;
uniform float roughness;
uniform float metalness;
uniform float opacity;
#ifdef IOR
	uniform float ior;
#endif
#ifdef USE_SPECULAR
	uniform float specularIntensity;
	uniform vec3 specularColor;
	#ifdef USE_SPECULAR_COLORMAP
		uniform sampler2D specularColorMap;
	#endif
	#ifdef USE_SPECULAR_INTENSITYMAP
		uniform sampler2D specularIntensityMap;
	#endif
#endif
#ifdef USE_CLEARCOAT
	uniform float clearcoat;
	uniform float clearcoatRoughness;
#endif
#ifdef USE_IRIDESCENCE
	uniform float iridescence;
	uniform float iridescenceIOR;
	uniform float iridescenceThicknessMinimum;
	uniform float iridescenceThicknessMaximum;
#endif
#ifdef USE_SHEEN
	uniform vec3 sheenColor;
	uniform float sheenRoughness;
	#ifdef USE_SHEEN_COLORMAP
		uniform sampler2D sheenColorMap;
	#endif
	#ifdef USE_SHEEN_ROUGHNESSMAP
		uniform sampler2D sheenRoughnessMap;
	#endif
#endif
#ifdef USE_ANISOTROPY
	uniform vec2 anisotropyVector;
	#ifdef USE_ANISOTROPYMAP
		uniform sampler2D anisotropyMap;
	#endif
#endif
varying vec3 vViewPosition;
#include <common>
#include <packing>
#include <dithering_pars_fragment>
#include <color_pars_fragment>
#include <uv_pars_fragment>
#include <map_pars_fragment>
#include <alphamap_pars_fragment>
#include <alphatest_pars_fragment>
#include <alphahash_pars_fragment>
#include <aomap_pars_fragment>
#include <lightmap_pars_fragment>
#include <emissivemap_pars_fragment>
#include <iridescence_fragment>
#include <cube_uv_reflection_fragment>
#include <envmap_common_pars_fragment>
#include <envmap_physical_pars_fragment>
#include <fog_pars_fragment>
#include <lights_pars_begin>
#include <normal_pars_fragment>
#include <lights_physical_pars_fragment>
#include <transmission_pars_fragment>
#include <shadowmap_pars_fragment>
#include <bumpmap_pars_fragment>
#include <normalmap_pars_fragment>
#include <clearcoat_pars_fragment>
#include <iridescence_pars_fragment>
#include <roughnessmap_pars_fragment>
#include <metalnessmap_pars_fragment>
#include <logdepthbuf_pars_fragment>
#include <clipping_planes_pars_fragment>
void main() {
	#include <clipping_planes_fragment>
	vec4 diffuseColor = vec4( diffuse, opacity );
	ReflectedLight reflectedLight = ReflectedLight( vec3( 0.0 ), vec3( 0.0 ), vec3( 0.0 ), vec3( 0.0 ) );
	vec3 totalEmissiveRadiance = emissive;
	#include <logdepthbuf_fragment>
	#include <map_fragment>
	#include <color_fragment>
	#include <alphamap_fragment>
	#include <alphatest_fragment>
	#include <alphahash_fragment>
	#include <roughnessmap_fragment>
	#include <metalnessmap_fragment>
	#include <normal_fragment_begin>
	#include <normal_fragment_maps>
	#include <clearcoat_normal_fragment_begin>
	#include <clearcoat_normal_fragment_maps>
	#include <emissivemap_fragment>
	#include <lights_physical_fragment>
	#include <lights_fragment_begin>
	#include <lights_fragment_maps>
	#include <lights_fragment_end>
	#include <aomap_fragment>
	vec3 totalDiffuse = reflectedLight.directDiffuse + reflectedLight.indirectDiffuse;
	vec3 totalSpecular = reflectedLight.directSpecular + reflectedLight.indirectSpecular;
	#include <transmission_fragment>
	vec3 outgoingLight = totalDiffuse + totalSpecular + totalEmissiveRadiance;
	#ifdef USE_SHEEN
		float sheenEnergyComp = 1.0 - 0.157 * max3( material.sheenColor );
		outgoingLight = outgoingLight * sheenEnergyComp + sheenSpecularDirect + sheenSpecularIndirect;
	#endif
	#ifdef USE_CLEARCOAT
		float dotNVcc = saturate( dot( geometryClearcoatNormal, geometryViewDir ) );
		vec3 Fcc = F_Schlick( material.clearcoatF0, material.clearcoatF90, dotNVcc );
		outgoingLight = outgoingLight * ( 1.0 - material.clearcoat * Fcc ) + ( clearcoatSpecularDirect + clearcoatSpecularIndirect ) * material.clearcoat;
	#endif
	#include <opaque_fragment>
	#include <tonemapping_fragment>
	#include <colorspace_fragment>
	#include <fog_fragment>
	#include <premultiplied_alpha_fragment>
	#include <dithering_fragment>
}`,xy=`#define TOON
varying vec3 vViewPosition;
#include <common>
#include <batching_pars_vertex>
#include <uv_pars_vertex>
#include <displacementmap_pars_vertex>
#include <color_pars_vertex>
#include <fog_pars_vertex>
#include <normal_pars_vertex>
#include <morphtarget_pars_vertex>
#include <skinning_pars_vertex>
#include <shadowmap_pars_vertex>
#include <logdepthbuf_pars_vertex>
#include <clipping_planes_pars_vertex>
void main() {
	#include <uv_vertex>
	#include <color_vertex>
	#include <morphcolor_vertex>
	#include <batching_vertex>
	#include <beginnormal_vertex>
	#include <morphnormal_vertex>
	#include <skinbase_vertex>
	#include <skinnormal_vertex>
	#include <defaultnormal_vertex>
	#include <normal_vertex>
	#include <begin_vertex>
	#include <morphtarget_vertex>
	#include <skinning_vertex>
	#include <displacementmap_vertex>
	#include <project_vertex>
	#include <logdepthbuf_vertex>
	#include <clipping_planes_vertex>
	vViewPosition = - mvPosition.xyz;
	#include <worldpos_vertex>
	#include <shadowmap_vertex>
	#include <fog_vertex>
}`,Sy=`#define TOON
uniform vec3 diffuse;
uniform vec3 emissive;
uniform float opacity;
#include <common>
#include <packing>
#include <dithering_pars_fragment>
#include <color_pars_fragment>
#include <uv_pars_fragment>
#include <map_pars_fragment>
#include <alphamap_pars_fragment>
#include <alphatest_pars_fragment>
#include <alphahash_pars_fragment>
#include <aomap_pars_fragment>
#include <lightmap_pars_fragment>
#include <emissivemap_pars_fragment>
#include <gradientmap_pars_fragment>
#include <fog_pars_fragment>
#include <bsdfs>
#include <lights_pars_begin>
#include <normal_pars_fragment>
#include <lights_toon_pars_fragment>
#include <shadowmap_pars_fragment>
#include <bumpmap_pars_fragment>
#include <normalmap_pars_fragment>
#include <logdepthbuf_pars_fragment>
#include <clipping_planes_pars_fragment>
void main() {
	#include <clipping_planes_fragment>
	vec4 diffuseColor = vec4( diffuse, opacity );
	ReflectedLight reflectedLight = ReflectedLight( vec3( 0.0 ), vec3( 0.0 ), vec3( 0.0 ), vec3( 0.0 ) );
	vec3 totalEmissiveRadiance = emissive;
	#include <logdepthbuf_fragment>
	#include <map_fragment>
	#include <color_fragment>
	#include <alphamap_fragment>
	#include <alphatest_fragment>
	#include <alphahash_fragment>
	#include <normal_fragment_begin>
	#include <normal_fragment_maps>
	#include <emissivemap_fragment>
	#include <lights_toon_fragment>
	#include <lights_fragment_begin>
	#include <lights_fragment_maps>
	#include <lights_fragment_end>
	#include <aomap_fragment>
	vec3 outgoingLight = reflectedLight.directDiffuse + reflectedLight.indirectDiffuse + totalEmissiveRadiance;
	#include <opaque_fragment>
	#include <tonemapping_fragment>
	#include <colorspace_fragment>
	#include <fog_fragment>
	#include <premultiplied_alpha_fragment>
	#include <dithering_fragment>
}`,Ty=`uniform float size;
uniform float scale;
#include <common>
#include <color_pars_vertex>
#include <fog_pars_vertex>
#include <morphtarget_pars_vertex>
#include <logdepthbuf_pars_vertex>
#include <clipping_planes_pars_vertex>
#ifdef USE_POINTS_UV
	varying vec2 vUv;
	uniform mat3 uvTransform;
#endif
void main() {
	#ifdef USE_POINTS_UV
		vUv = ( uvTransform * vec3( uv, 1 ) ).xy;
	#endif
	#include <color_vertex>
	#include <morphcolor_vertex>
	#include <begin_vertex>
	#include <morphtarget_vertex>
	#include <project_vertex>
	gl_PointSize = size;
	#ifdef USE_SIZEATTENUATION
		bool isPerspective = isPerspectiveMatrix( projectionMatrix );
		if ( isPerspective ) gl_PointSize *= ( scale / - mvPosition.z );
	#endif
	#include <logdepthbuf_vertex>
	#include <clipping_planes_vertex>
	#include <worldpos_vertex>
	#include <fog_vertex>
}`,My=`uniform vec3 diffuse;
uniform float opacity;
#include <common>
#include <color_pars_fragment>
#include <map_particle_pars_fragment>
#include <alphatest_pars_fragment>
#include <alphahash_pars_fragment>
#include <fog_pars_fragment>
#include <logdepthbuf_pars_fragment>
#include <clipping_planes_pars_fragment>
void main() {
	#include <clipping_planes_fragment>
	vec3 outgoingLight = vec3( 0.0 );
	vec4 diffuseColor = vec4( diffuse, opacity );
	#include <logdepthbuf_fragment>
	#include <map_particle_fragment>
	#include <color_fragment>
	#include <alphatest_fragment>
	#include <alphahash_fragment>
	outgoingLight = diffuseColor.rgb;
	#include <opaque_fragment>
	#include <tonemapping_fragment>
	#include <colorspace_fragment>
	#include <fog_fragment>
	#include <premultiplied_alpha_fragment>
}`,wy=`#include <common>
#include <batching_pars_vertex>
#include <fog_pars_vertex>
#include <morphtarget_pars_vertex>
#include <skinning_pars_vertex>
#include <logdepthbuf_pars_vertex>
#include <shadowmap_pars_vertex>
void main() {
	#include <batching_vertex>
	#include <beginnormal_vertex>
	#include <morphnormal_vertex>
	#include <skinbase_vertex>
	#include <skinnormal_vertex>
	#include <defaultnormal_vertex>
	#include <begin_vertex>
	#include <morphtarget_vertex>
	#include <skinning_vertex>
	#include <project_vertex>
	#include <logdepthbuf_vertex>
	#include <worldpos_vertex>
	#include <shadowmap_vertex>
	#include <fog_vertex>
}`,by=`uniform vec3 color;
uniform float opacity;
#include <common>
#include <packing>
#include <fog_pars_fragment>
#include <bsdfs>
#include <lights_pars_begin>
#include <logdepthbuf_pars_fragment>
#include <shadowmap_pars_fragment>
#include <shadowmask_pars_fragment>
void main() {
	#include <logdepthbuf_fragment>
	gl_FragColor = vec4( color, opacity * ( 1.0 - getShadowMask() ) );
	#include <tonemapping_fragment>
	#include <colorspace_fragment>
	#include <fog_fragment>
}`,Ay=`uniform float rotation;
uniform vec2 center;
#include <common>
#include <uv_pars_vertex>
#include <fog_pars_vertex>
#include <logdepthbuf_pars_vertex>
#include <clipping_planes_pars_vertex>
void main() {
	#include <uv_vertex>
	vec4 mvPosition = modelViewMatrix * vec4( 0.0, 0.0, 0.0, 1.0 );
	vec2 scale;
	scale.x = length( vec3( modelMatrix[ 0 ].x, modelMatrix[ 0 ].y, modelMatrix[ 0 ].z ) );
	scale.y = length( vec3( modelMatrix[ 1 ].x, modelMatrix[ 1 ].y, modelMatrix[ 1 ].z ) );
	#ifndef USE_SIZEATTENUATION
		bool isPerspective = isPerspectiveMatrix( projectionMatrix );
		if ( isPerspective ) scale *= - mvPosition.z;
	#endif
	vec2 alignedPosition = ( position.xy - ( center - vec2( 0.5 ) ) ) * scale;
	vec2 rotatedPosition;
	rotatedPosition.x = cos( rotation ) * alignedPosition.x - sin( rotation ) * alignedPosition.y;
	rotatedPosition.y = sin( rotation ) * alignedPosition.x + cos( rotation ) * alignedPosition.y;
	mvPosition.xy += rotatedPosition;
	gl_Position = projectionMatrix * mvPosition;
	#include <logdepthbuf_vertex>
	#include <clipping_planes_vertex>
	#include <fog_vertex>
}`,Ry=`uniform vec3 diffuse;
uniform float opacity;
#include <common>
#include <uv_pars_fragment>
#include <map_pars_fragment>
#include <alphamap_pars_fragment>
#include <alphatest_pars_fragment>
#include <alphahash_pars_fragment>
#include <fog_pars_fragment>
#include <logdepthbuf_pars_fragment>
#include <clipping_planes_pars_fragment>
void main() {
	#include <clipping_planes_fragment>
	vec3 outgoingLight = vec3( 0.0 );
	vec4 diffuseColor = vec4( diffuse, opacity );
	#include <logdepthbuf_fragment>
	#include <map_fragment>
	#include <alphamap_fragment>
	#include <alphatest_fragment>
	#include <alphahash_fragment>
	outgoingLight = diffuseColor.rgb;
	#include <opaque_fragment>
	#include <tonemapping_fragment>
	#include <colorspace_fragment>
	#include <fog_fragment>
}`,re={alphahash_fragment:K_,alphahash_pars_fragment:Q_,alphamap_fragment:Z_,alphamap_pars_fragment:J_,alphatest_fragment:tv,alphatest_pars_fragment:ev,aomap_fragment:nv,aomap_pars_fragment:iv,batching_pars_vertex:rv,batching_vertex:sv,begin_vertex:ov,beginnormal_vertex:av,bsdfs:cv,iridescence_fragment:lv,bumpmap_pars_fragment:hv,clipping_planes_fragment:uv,clipping_planes_pars_fragment:dv,clipping_planes_pars_vertex:fv,clipping_planes_vertex:pv,color_fragment:mv,color_pars_fragment:gv,color_pars_vertex:_v,color_vertex:vv,common:yv,cube_uv_reflection_fragment:Ev,defaultnormal_vertex:xv,displacementmap_pars_vertex:Sv,displacementmap_vertex:Tv,emissivemap_fragment:Mv,emissivemap_pars_fragment:wv,colorspace_fragment:bv,colorspace_pars_fragment:Av,envmap_fragment:Rv,envmap_common_pars_fragment:Cv,envmap_pars_fragment:Iv,envmap_pars_vertex:Pv,envmap_physical_pars_fragment:Gv,envmap_vertex:Dv,fog_vertex:Lv,fog_pars_vertex:Nv,fog_fragment:Uv,fog_pars_fragment:Ov,gradientmap_pars_fragment:Fv,lightmap_fragment:Vv,lightmap_pars_fragment:Bv,lights_lambert_fragment:kv,lights_lambert_pars_fragment:Hv,lights_pars_begin:zv,lights_toon_fragment:Wv,lights_toon_pars_fragment:qv,lights_phong_fragment:Xv,lights_phong_pars_fragment:jv,lights_physical_fragment:Yv,lights_physical_pars_fragment:$v,lights_fragment_begin:Kv,lights_fragment_maps:Qv,lights_fragment_end:Zv,logdepthbuf_fragment:Jv,logdepthbuf_pars_fragment:t0,logdepthbuf_pars_vertex:e0,logdepthbuf_vertex:n0,map_fragment:i0,map_pars_fragment:r0,map_particle_fragment:s0,map_particle_pars_fragment:o0,metalnessmap_fragment:a0,metalnessmap_pars_fragment:c0,morphcolor_vertex:l0,morphnormal_vertex:h0,morphtarget_pars_vertex:u0,morphtarget_vertex:d0,normal_fragment_begin:f0,normal_fragment_maps:p0,normal_pars_fragment:m0,normal_pars_vertex:g0,normal_vertex:_0,normalmap_pars_fragment:v0,clearcoat_normal_fragment_begin:y0,clearcoat_normal_fragment_maps:E0,clearcoat_pars_fragment:x0,iridescence_pars_fragment:S0,opaque_fragment:T0,packing:M0,premultiplied_alpha_fragment:w0,project_vertex:b0,dithering_fragment:A0,dithering_pars_fragment:R0,roughnessmap_fragment:C0,roughnessmap_pars_fragment:I0,shadowmap_pars_fragment:P0,shadowmap_pars_vertex:D0,shadowmap_vertex:L0,shadowmask_pars_fragment:N0,skinbase_vertex:U0,skinning_pars_vertex:O0,skinning_vertex:F0,skinnormal_vertex:V0,specularmap_fragment:B0,specularmap_pars_fragment:k0,tonemapping_fragment:H0,tonemapping_pars_fragment:z0,transmission_fragment:G0,transmission_pars_fragment:W0,uv_pars_fragment:q0,uv_pars_vertex:X0,uv_vertex:j0,worldpos_vertex:Y0,background_vert:$0,background_frag:K0,backgroundCube_vert:Q0,backgroundCube_frag:Z0,cube_vert:J0,cube_frag:ty,depth_vert:ey,depth_frag:ny,distanceRGBA_vert:iy,distanceRGBA_frag:ry,equirect_vert:sy,equirect_frag:oy,linedashed_vert:ay,linedashed_frag:cy,meshbasic_vert:ly,meshbasic_frag:hy,meshlambert_vert:uy,meshlambert_frag:dy,meshmatcap_vert:fy,meshmatcap_frag:py,meshnormal_vert:my,meshnormal_frag:gy,meshphong_vert:_y,meshphong_frag:vy,meshphysical_vert:yy,meshphysical_frag:Ey,meshtoon_vert:xy,meshtoon_frag:Sy,points_vert:Ty,points_frag:My,shadow_vert:wy,shadow_frag:by,sprite_vert:Ay,sprite_frag:Ry},vt={common:{diffuse:{value:new oe(16777215)},opacity:{value:1},map:{value:null},mapTransform:{value:new ae},alphaMap:{value:null},alphaMapTransform:{value:new ae},alphaTest:{value:0}},specularmap:{specularMap:{value:null},specularMapTransform:{value:new ae}},envmap:{envMap:{value:null},flipEnvMap:{value:-1},reflectivity:{value:1},ior:{value:1.5},refractionRatio:{value:.98}},aomap:{aoMap:{value:null},aoMapIntensity:{value:1},aoMapTransform:{value:new ae}},lightmap:{lightMap:{value:null},lightMapIntensity:{value:1},lightMapTransform:{value:new ae}},bumpmap:{bumpMap:{value:null},bumpMapTransform:{value:new ae},bumpScale:{value:1}},normalmap:{normalMap:{value:null},normalMapTransform:{value:new ae},normalScale:{value:new Vt(1,1)}},displacementmap:{displacementMap:{value:null},displacementMapTransform:{value:new ae},displacementScale:{value:1},displacementBias:{value:0}},emissivemap:{emissiveMap:{value:null},emissiveMapTransform:{value:new ae}},metalnessmap:{metalnessMap:{value:null},metalnessMapTransform:{value:new ae}},roughnessmap:{roughnessMap:{value:null},roughnessMapTransform:{value:new ae}},gradientmap:{gradientMap:{value:null}},fog:{fogDensity:{value:25e-5},fogNear:{value:1},fogFar:{value:2e3},fogColor:{value:new oe(16777215)}},lights:{ambientLightColor:{value:[]},lightProbe:{value:[]},directionalLights:{value:[],properties:{direction:{},color:{}}},directionalLightShadows:{value:[],properties:{shadowBias:{},shadowNormalBias:{},shadowRadius:{},shadowMapSize:{}}},directionalShadowMap:{value:[]},directionalShadowMatrix:{value:[]},spotLights:{value:[],properties:{color:{},position:{},direction:{},distance:{},coneCos:{},penumbraCos:{},decay:{}}},spotLightShadows:{value:[],properties:{shadowBias:{},shadowNormalBias:{},shadowRadius:{},shadowMapSize:{}}},spotLightMap:{value:[]},spotShadowMap:{value:[]},spotLightMatrix:{value:[]},pointLights:{value:[],properties:{color:{},position:{},decay:{},distance:{}}},pointLightShadows:{value:[],properties:{shadowBias:{},shadowNormalBias:{},shadowRadius:{},shadowMapSize:{},shadowCameraNear:{},shadowCameraFar:{}}},pointShadowMap:{value:[]},pointShadowMatrix:{value:[]},hemisphereLights:{value:[],properties:{direction:{},skyColor:{},groundColor:{}}},rectAreaLights:{value:[],properties:{color:{},position:{},width:{},height:{}}},ltc_1:{value:null},ltc_2:{value:null}},points:{diffuse:{value:new oe(16777215)},opacity:{value:1},size:{value:1},scale:{value:1},map:{value:null},alphaMap:{value:null},alphaMapTransform:{value:new ae},alphaTest:{value:0},uvTransform:{value:new ae}},sprite:{diffuse:{value:new oe(16777215)},opacity:{value:1},center:{value:new Vt(.5,.5)},rotation:{value:0},map:{value:null},mapTransform:{value:new ae},alphaMap:{value:null},alphaMapTransform:{value:new ae},alphaTest:{value:0}}},$n={basic:{uniforms:fn([vt.common,vt.specularmap,vt.envmap,vt.aomap,vt.lightmap,vt.fog]),vertexShader:re.meshbasic_vert,fragmentShader:re.meshbasic_frag},lambert:{uniforms:fn([vt.common,vt.specularmap,vt.envmap,vt.aomap,vt.lightmap,vt.emissivemap,vt.bumpmap,vt.normalmap,vt.displacementmap,vt.fog,vt.lights,{emissive:{value:new oe(0)}}]),vertexShader:re.meshlambert_vert,fragmentShader:re.meshlambert_frag},phong:{uniforms:fn([vt.common,vt.specularmap,vt.envmap,vt.aomap,vt.lightmap,vt.emissivemap,vt.bumpmap,vt.normalmap,vt.displacementmap,vt.fog,vt.lights,{emissive:{value:new oe(0)},specular:{value:new oe(1118481)},shininess:{value:30}}]),vertexShader:re.meshphong_vert,fragmentShader:re.meshphong_frag},standard:{uniforms:fn([vt.common,vt.envmap,vt.aomap,vt.lightmap,vt.emissivemap,vt.bumpmap,vt.normalmap,vt.displacementmap,vt.roughnessmap,vt.metalnessmap,vt.fog,vt.lights,{emissive:{value:new oe(0)},roughness:{value:1},metalness:{value:0},envMapIntensity:{value:1}}]),vertexShader:re.meshphysical_vert,fragmentShader:re.meshphysical_frag},toon:{uniforms:fn([vt.common,vt.aomap,vt.lightmap,vt.emissivemap,vt.bumpmap,vt.normalmap,vt.displacementmap,vt.gradientmap,vt.fog,vt.lights,{emissive:{value:new oe(0)}}]),vertexShader:re.meshtoon_vert,fragmentShader:re.meshtoon_frag},matcap:{uniforms:fn([vt.common,vt.bumpmap,vt.normalmap,vt.displacementmap,vt.fog,{matcap:{value:null}}]),vertexShader:re.meshmatcap_vert,fragmentShader:re.meshmatcap_frag},points:{uniforms:fn([vt.points,vt.fog]),vertexShader:re.points_vert,fragmentShader:re.points_frag},dashed:{uniforms:fn([vt.common,vt.fog,{scale:{value:1},dashSize:{value:1},totalSize:{value:2}}]),vertexShader:re.linedashed_vert,fragmentShader:re.linedashed_frag},depth:{uniforms:fn([vt.common,vt.displacementmap]),vertexShader:re.depth_vert,fragmentShader:re.depth_frag},normal:{uniforms:fn([vt.common,vt.bumpmap,vt.normalmap,vt.displacementmap,{opacity:{value:1}}]),vertexShader:re.meshnormal_vert,fragmentShader:re.meshnormal_frag},sprite:{uniforms:fn([vt.sprite,vt.fog]),vertexShader:re.sprite_vert,fragmentShader:re.sprite_frag},background:{uniforms:{uvTransform:{value:new ae},t2D:{value:null},backgroundIntensity:{value:1}},vertexShader:re.background_vert,fragmentShader:re.background_frag},backgroundCube:{uniforms:{envMap:{value:null},flipEnvMap:{value:-1},backgroundBlurriness:{value:0},backgroundIntensity:{value:1}},vertexShader:re.backgroundCube_vert,fragmentShader:re.backgroundCube_frag},cube:{uniforms:{tCube:{value:null},tFlip:{value:-1},opacity:{value:1}},vertexShader:re.cube_vert,fragmentShader:re.cube_frag},equirect:{uniforms:{tEquirect:{value:null}},vertexShader:re.equirect_vert,fragmentShader:re.equirect_frag},distanceRGBA:{uniforms:fn([vt.common,vt.displacementmap,{referencePosition:{value:new k},nearDistance:{value:1},farDistance:{value:1e3}}]),vertexShader:re.distanceRGBA_vert,fragmentShader:re.distanceRGBA_frag},shadow:{uniforms:fn([vt.lights,vt.fog,{color:{value:new oe(0)},opacity:{value:1}}]),vertexShader:re.shadow_vert,fragmentShader:re.shadow_frag}};$n.physical={uniforms:fn([$n.standard.uniforms,{clearcoat:{value:0},clearcoatMap:{value:null},clearcoatMapTransform:{value:new ae},clearcoatNormalMap:{value:null},clearcoatNormalMapTransform:{value:new ae},clearcoatNormalScale:{value:new Vt(1,1)},clearcoatRoughness:{value:0},clearcoatRoughnessMap:{value:null},clearcoatRoughnessMapTransform:{value:new ae},iridescence:{value:0},iridescenceMap:{value:null},iridescenceMapTransform:{value:new ae},iridescenceIOR:{value:1.3},iridescenceThicknessMinimum:{value:100},iridescenceThicknessMaximum:{value:400},iridescenceThicknessMap:{value:null},iridescenceThicknessMapTransform:{value:new ae},sheen:{value:0},sheenColor:{value:new oe(0)},sheenColorMap:{value:null},sheenColorMapTransform:{value:new ae},sheenRoughness:{value:1},sheenRoughnessMap:{value:null},sheenRoughnessMapTransform:{value:new ae},transmission:{value:0},transmissionMap:{value:null},transmissionMapTransform:{value:new ae},transmissionSamplerSize:{value:new Vt},transmissionSamplerMap:{value:null},thickness:{value:0},thicknessMap:{value:null},thicknessMapTransform:{value:new ae},attenuationDistance:{value:0},attenuationColor:{value:new oe(0)},specularColor:{value:new oe(1,1,1)},specularColorMap:{value:null},specularColorMapTransform:{value:new ae},specularIntensity:{value:1},specularIntensityMap:{value:null},specularIntensityMapTransform:{value:new ae},anisotropyVector:{value:new Vt},anisotropyMap:{value:null},anisotropyMapTransform:{value:new ae}}]),vertexShader:re.meshphysical_vert,fragmentShader:re.meshphysical_frag};const Jo={r:0,b:0,g:0};function Cy(i,t,e,n,r,s,o){const a=new oe(0);let c=s===!0?0:1,h,u,d=null,f=0,p=null;function y(_,m){let C=!1,w=m.isScene===!0?m.background:null;w&&w.isTexture&&(w=(m.backgroundBlurriness>0?e:t).get(w)),w===null?E(a,c):w&&w.isColor&&(E(w,1),C=!0);const b=i.xr.getEnvironmentBlendMode();b==="additive"?n.buffers.color.setClear(0,0,0,1,o):b==="alpha-blend"&&n.buffers.color.setClear(0,0,0,0,o),(i.autoClear||C)&&i.clear(i.autoClearColor,i.autoClearDepth,i.autoClearStencil),w&&(w.isCubeTexture||w.mapping===Ga)?(u===void 0&&(u=new Ie(new _s(1,1,1),new fi({name:"BackgroundCubeMaterial",uniforms:as($n.backgroundCube.uniforms),vertexShader:$n.backgroundCube.vertexShader,fragmentShader:$n.backgroundCube.fragmentShader,side:vn,depthTest:!1,depthWrite:!1,fog:!1})),u.geometry.deleteAttribute("normal"),u.geometry.deleteAttribute("uv"),u.onBeforeRender=function(N,U,T){this.matrixWorld.copyPosition(T.matrixWorld)},Object.defineProperty(u.material,"envMap",{get:function(){return this.uniforms.envMap.value}}),r.update(u)),u.material.uniforms.envMap.value=w,u.material.uniforms.flipEnvMap.value=w.isCubeTexture&&w.isRenderTargetTexture===!1?-1:1,u.material.uniforms.backgroundBlurriness.value=m.backgroundBlurriness,u.material.uniforms.backgroundIntensity.value=m.backgroundIntensity,u.material.toneMapped=ve.getTransfer(w.colorSpace)!==we,(d!==w||f!==w.version||p!==i.toneMapping)&&(u.material.needsUpdate=!0,d=w,f=w.version,p=i.toneMapping),u.layers.enableAll(),_.unshift(u,u.geometry,u.material,0,0,null)):w&&w.isTexture&&(h===void 0&&(h=new Ie(new dr(2,2),new fi({name:"BackgroundMaterial",uniforms:as($n.background.uniforms),vertexShader:$n.background.vertexShader,fragmentShader:$n.background.fragmentShader,side:ui,depthTest:!1,depthWrite:!1,fog:!1})),h.geometry.deleteAttribute("normal"),Object.defineProperty(h.material,"map",{get:function(){return this.uniforms.t2D.value}}),r.update(h)),h.material.uniforms.t2D.value=w,h.material.uniforms.backgroundIntensity.value=m.backgroundIntensity,h.material.toneMapped=ve.getTransfer(w.colorSpace)!==we,w.matrixAutoUpdate===!0&&w.updateMatrix(),h.material.uniforms.uvTransform.value.copy(w.matrix),(d!==w||f!==w.version||p!==i.toneMapping)&&(h.material.needsUpdate=!0,d=w,f=w.version,p=i.toneMapping),h.layers.enableAll(),_.unshift(h,h.geometry,h.material,0,0,null))}function E(_,m){_.getRGB(Jo,up(i)),n.buffers.color.setClear(Jo.r,Jo.g,Jo.b,m,o)}return{getClearColor:function(){return a},setClearColor:function(_,m=1){a.set(_),c=m,E(a,c)},getClearAlpha:function(){return c},setClearAlpha:function(_){c=_,E(a,c)},render:y}}function Iy(i,t,e,n){const r=i.getParameter(i.MAX_VERTEX_ATTRIBS),s=n.isWebGL2?null:t.get("OES_vertex_array_object"),o=n.isWebGL2||s!==null,a={},c=_(null);let h=c,u=!1;function d(V,G,K,Z,J){let et=!1;if(o){const tt=E(Z,K,G);h!==tt&&(h=tt,p(h.object)),et=m(V,Z,K,J),et&&C(V,Z,K,J)}else{const tt=G.wireframe===!0;(h.geometry!==Z.id||h.program!==K.id||h.wireframe!==tt)&&(h.geometry=Z.id,h.program=K.id,h.wireframe=tt,et=!0)}J!==null&&e.update(J,i.ELEMENT_ARRAY_BUFFER),(et||u)&&(u=!1,M(V,G,K,Z),J!==null&&i.bindBuffer(i.ELEMENT_ARRAY_BUFFER,e.get(J).buffer))}function f(){return n.isWebGL2?i.createVertexArray():s.createVertexArrayOES()}function p(V){return n.isWebGL2?i.bindVertexArray(V):s.bindVertexArrayOES(V)}function y(V){return n.isWebGL2?i.deleteVertexArray(V):s.deleteVertexArrayOES(V)}function E(V,G,K){const Z=K.wireframe===!0;let J=a[V.id];J===void 0&&(J={},a[V.id]=J);let et=J[G.id];et===void 0&&(et={},J[G.id]=et);let tt=et[Z];return tt===void 0&&(tt=_(f()),et[Z]=tt),tt}function _(V){const G=[],K=[],Z=[];for(let J=0;J<r;J++)G[J]=0,K[J]=0,Z[J]=0;return{geometry:null,program:null,wireframe:!1,newAttributes:G,enabledAttributes:K,attributeDivisors:Z,object:V,attributes:{},index:null}}function m(V,G,K,Z){const J=h.attributes,et=G.attributes;let tt=0;const pt=K.getAttributes();for(const ht in pt)if(pt[ht].location>=0){const it=J[ht];let mt=et[ht];if(mt===void 0&&(ht==="instanceMatrix"&&V.instanceMatrix&&(mt=V.instanceMatrix),ht==="instanceColor"&&V.instanceColor&&(mt=V.instanceColor)),it===void 0||it.attribute!==mt||mt&&it.data!==mt.data)return!0;tt++}return h.attributesNum!==tt||h.index!==Z}function C(V,G,K,Z){const J={},et=G.attributes;let tt=0;const pt=K.getAttributes();for(const ht in pt)if(pt[ht].location>=0){let it=et[ht];it===void 0&&(ht==="instanceMatrix"&&V.instanceMatrix&&(it=V.instanceMatrix),ht==="instanceColor"&&V.instanceColor&&(it=V.instanceColor));const mt={};mt.attribute=it,it&&it.data&&(mt.data=it.data),J[ht]=mt,tt++}h.attributes=J,h.attributesNum=tt,h.index=Z}function w(){const V=h.newAttributes;for(let G=0,K=V.length;G<K;G++)V[G]=0}function b(V){N(V,0)}function N(V,G){const K=h.newAttributes,Z=h.enabledAttributes,J=h.attributeDivisors;K[V]=1,Z[V]===0&&(i.enableVertexAttribArray(V),Z[V]=1),J[V]!==G&&((n.isWebGL2?i:t.get("ANGLE_instanced_arrays"))[n.isWebGL2?"vertexAttribDivisor":"vertexAttribDivisorANGLE"](V,G),J[V]=G)}function U(){const V=h.newAttributes,G=h.enabledAttributes;for(let K=0,Z=G.length;K<Z;K++)G[K]!==V[K]&&(i.disableVertexAttribArray(K),G[K]=0)}function T(V,G,K,Z,J,et,tt){tt===!0?i.vertexAttribIPointer(V,G,K,J,et):i.vertexAttribPointer(V,G,K,Z,J,et)}function M(V,G,K,Z){if(n.isWebGL2===!1&&(V.isInstancedMesh||Z.isInstancedBufferGeometry)&&t.get("ANGLE_instanced_arrays")===null)return;w();const J=Z.attributes,et=K.getAttributes(),tt=G.defaultAttributeValues;for(const pt in et){const ht=et[pt];if(ht.location>=0){let X=J[pt];if(X===void 0&&(pt==="instanceMatrix"&&V.instanceMatrix&&(X=V.instanceMatrix),pt==="instanceColor"&&V.instanceColor&&(X=V.instanceColor)),X!==void 0){const it=X.normalized,mt=X.itemSize,Pt=e.get(X);if(Pt===void 0)continue;const Mt=Pt.buffer,Gt=Pt.type,Ft=Pt.bytesPerElement,wt=n.isWebGL2===!0&&(Gt===i.INT||Gt===i.UNSIGNED_INT||X.gpuType===Yf);if(X.isInterleavedBufferAttribute){const Wt=X.data,W=Wt.stride,pe=X.offset;if(Wt.isInstancedInterleavedBuffer){for(let Rt=0;Rt<ht.locationSize;Rt++)N(ht.location+Rt,Wt.meshPerAttribute);V.isInstancedMesh!==!0&&Z._maxInstanceCount===void 0&&(Z._maxInstanceCount=Wt.meshPerAttribute*Wt.count)}else for(let Rt=0;Rt<ht.locationSize;Rt++)b(ht.location+Rt);i.bindBuffer(i.ARRAY_BUFFER,Mt);for(let Rt=0;Rt<ht.locationSize;Rt++)T(ht.location+Rt,mt/ht.locationSize,Gt,it,W*Ft,(pe+mt/ht.locationSize*Rt)*Ft,wt)}else{if(X.isInstancedBufferAttribute){for(let Wt=0;Wt<ht.locationSize;Wt++)N(ht.location+Wt,X.meshPerAttribute);V.isInstancedMesh!==!0&&Z._maxInstanceCount===void 0&&(Z._maxInstanceCount=X.meshPerAttribute*X.count)}else for(let Wt=0;Wt<ht.locationSize;Wt++)b(ht.location+Wt);i.bindBuffer(i.ARRAY_BUFFER,Mt);for(let Wt=0;Wt<ht.locationSize;Wt++)T(ht.location+Wt,mt/ht.locationSize,Gt,it,mt*Ft,mt/ht.locationSize*Wt*Ft,wt)}}else if(tt!==void 0){const it=tt[pt];if(it!==void 0)switch(it.length){case 2:i.vertexAttrib2fv(ht.location,it);break;case 3:i.vertexAttrib3fv(ht.location,it);break;case 4:i.vertexAttrib4fv(ht.location,it);break;default:i.vertexAttrib1fv(ht.location,it)}}}}U()}function v(){L();for(const V in a){const G=a[V];for(const K in G){const Z=G[K];for(const J in Z)y(Z[J].object),delete Z[J];delete G[K]}delete a[V]}}function S(V){if(a[V.id]===void 0)return;const G=a[V.id];for(const K in G){const Z=G[K];for(const J in Z)y(Z[J].object),delete Z[J];delete G[K]}delete a[V.id]}function P(V){for(const G in a){const K=a[G];if(K[V.id]===void 0)continue;const Z=K[V.id];for(const J in Z)y(Z[J].object),delete Z[J];delete K[V.id]}}function L(){R(),u=!0,h!==c&&(h=c,p(h.object))}function R(){c.geometry=null,c.program=null,c.wireframe=!1}return{setup:d,reset:L,resetDefaultState:R,dispose:v,releaseStatesOfGeometry:S,releaseStatesOfProgram:P,initAttributes:w,enableAttribute:b,disableUnusedAttributes:U}}function Py(i,t,e,n){const r=n.isWebGL2;let s;function o(u){s=u}function a(u,d){i.drawArrays(s,u,d),e.update(d,s,1)}function c(u,d,f){if(f===0)return;let p,y;if(r)p=i,y="drawArraysInstanced";else if(p=t.get("ANGLE_instanced_arrays"),y="drawArraysInstancedANGLE",p===null){console.error("THREE.WebGLBufferRenderer: using THREE.InstancedBufferGeometry but hardware does not support extension ANGLE_instanced_arrays.");return}p[y](s,u,d,f),e.update(d,s,f)}function h(u,d,f){if(f===0)return;const p=t.get("WEBGL_multi_draw");if(p===null)for(let y=0;y<f;y++)this.render(u[y],d[y]);else{p.multiDrawArraysWEBGL(s,u,0,d,0,f);let y=0;for(let E=0;E<f;E++)y+=d[E];e.update(y,s,1)}}this.setMode=o,this.render=a,this.renderInstances=c,this.renderMultiDraw=h}function Dy(i,t,e){let n;function r(){if(n!==void 0)return n;if(t.has("EXT_texture_filter_anisotropic")===!0){const T=t.get("EXT_texture_filter_anisotropic");n=i.getParameter(T.MAX_TEXTURE_MAX_ANISOTROPY_EXT)}else n=0;return n}function s(T){if(T==="highp"){if(i.getShaderPrecisionFormat(i.VERTEX_SHADER,i.HIGH_FLOAT).precision>0&&i.getShaderPrecisionFormat(i.FRAGMENT_SHADER,i.HIGH_FLOAT).precision>0)return"highp";T="mediump"}return T==="mediump"&&i.getShaderPrecisionFormat(i.VERTEX_SHADER,i.MEDIUM_FLOAT).precision>0&&i.getShaderPrecisionFormat(i.FRAGMENT_SHADER,i.MEDIUM_FLOAT).precision>0?"mediump":"lowp"}const o=typeof WebGL2RenderingContext<"u"&&i.constructor.name==="WebGL2RenderingContext";let a=e.precision!==void 0?e.precision:"highp";const c=s(a);c!==a&&(console.warn("THREE.WebGLRenderer:",a,"not supported, using",c,"instead."),a=c);const h=o||t.has("WEBGL_draw_buffers"),u=e.logarithmicDepthBuffer===!0,d=i.getParameter(i.MAX_TEXTURE_IMAGE_UNITS),f=i.getParameter(i.MAX_VERTEX_TEXTURE_IMAGE_UNITS),p=i.getParameter(i.MAX_TEXTURE_SIZE),y=i.getParameter(i.MAX_CUBE_MAP_TEXTURE_SIZE),E=i.getParameter(i.MAX_VERTEX_ATTRIBS),_=i.getParameter(i.MAX_VERTEX_UNIFORM_VECTORS),m=i.getParameter(i.MAX_VARYING_VECTORS),C=i.getParameter(i.MAX_FRAGMENT_UNIFORM_VECTORS),w=f>0,b=o||t.has("OES_texture_float"),N=w&&b,U=o?i.getParameter(i.MAX_SAMPLES):0;return{isWebGL2:o,drawBuffers:h,getMaxAnisotropy:r,getMaxPrecision:s,precision:a,logarithmicDepthBuffer:u,maxTextures:d,maxVertexTextures:f,maxTextureSize:p,maxCubemapSize:y,maxAttributes:E,maxVertexUniforms:_,maxVaryings:m,maxFragmentUniforms:C,vertexTextures:w,floatFragmentTextures:b,floatVertexTextures:N,maxSamples:U}}function Ly(i){const t=this;let e=null,n=0,r=!1,s=!1;const o=new bi,a=new ae,c={value:null,needsUpdate:!1};this.uniform=c,this.numPlanes=0,this.numIntersection=0,this.init=function(d,f){const p=d.length!==0||f||n!==0||r;return r=f,n=d.length,p},this.beginShadows=function(){s=!0,u(null)},this.endShadows=function(){s=!1},this.setGlobalState=function(d,f){e=u(d,f,0)},this.setState=function(d,f,p){const y=d.clippingPlanes,E=d.clipIntersection,_=d.clipShadows,m=i.get(d);if(!r||y===null||y.length===0||s&&!_)s?u(null):h();else{const C=s?0:n,w=C*4;let b=m.clippingState||null;c.value=b,b=u(y,f,w,p);for(let N=0;N!==w;++N)b[N]=e[N];m.clippingState=b,this.numIntersection=E?this.numPlanes:0,this.numPlanes+=C}};function h(){c.value!==e&&(c.value=e,c.needsUpdate=n>0),t.numPlanes=n,t.numIntersection=0}function u(d,f,p,y){const E=d!==null?d.length:0;let _=null;if(E!==0){if(_=c.value,y!==!0||_===null){const m=p+E*4,C=f.matrixWorldInverse;a.getNormalMatrix(C),(_===null||_.length<m)&&(_=new Float32Array(m));for(let w=0,b=p;w!==E;++w,b+=4)o.copy(d[w]).applyMatrix4(C,a),o.normal.toArray(_,b),_[b+3]=o.constant}c.value=_,c.needsUpdate=!0}return t.numPlanes=E,t.numIntersection=0,_}}function Ny(i){let t=new WeakMap;function e(o,a){return a===ol?o.mapping=rs:a===al&&(o.mapping=ss),o}function n(o){if(o&&o.isTexture){const a=o.mapping;if(a===ol||a===al)if(t.has(o)){const c=t.get(o).texture;return e(c,o.mapping)}else{const c=o.image;if(c&&c.height>0){const h=new X_(c.height/2);return h.fromEquirectangularTexture(i,o),t.set(o,h),o.addEventListener("dispose",r),e(h.texture,o.mapping)}else return null}}return o}function r(o){const a=o.target;a.removeEventListener("dispose",r);const c=t.get(a);c!==void 0&&(t.delete(a),c.dispose())}function s(){t=new WeakMap}return{get:n,dispose:s}}class mp extends dp{constructor(t=-1,e=1,n=1,r=-1,s=.1,o=2e3){super(),this.isOrthographicCamera=!0,this.type="OrthographicCamera",this.zoom=1,this.view=null,this.left=t,this.right=e,this.top=n,this.bottom=r,this.near=s,this.far=o,this.updateProjectionMatrix()}copy(t,e){return super.copy(t,e),this.left=t.left,this.right=t.right,this.top=t.top,this.bottom=t.bottom,this.near=t.near,this.far=t.far,this.zoom=t.zoom,this.view=t.view===null?null:Object.assign({},t.view),this}setViewOffset(t,e,n,r,s,o){this.view===null&&(this.view={enabled:!0,fullWidth:1,fullHeight:1,offsetX:0,offsetY:0,width:1,height:1}),this.view.enabled=!0,this.view.fullWidth=t,this.view.fullHeight=e,this.view.offsetX=n,this.view.offsetY=r,this.view.width=s,this.view.height=o,this.updateProjectionMatrix()}clearViewOffset(){this.view!==null&&(this.view.enabled=!1),this.updateProjectionMatrix()}updateProjectionMatrix(){const t=(this.right-this.left)/(2*this.zoom),e=(this.top-this.bottom)/(2*this.zoom),n=(this.right+this.left)/2,r=(this.top+this.bottom)/2;let s=n-t,o=n+t,a=r+e,c=r-e;if(this.view!==null&&this.view.enabled){const h=(this.right-this.left)/this.view.fullWidth/this.zoom,u=(this.top-this.bottom)/this.view.fullHeight/this.zoom;s+=h*this.view.offsetX,o=s+h*this.view.width,a-=u*this.view.offsetY,c=a-u*this.view.height}this.projectionMatrix.makeOrthographic(s,o,a,c,this.near,this.far,this.coordinateSystem),this.projectionMatrixInverse.copy(this.projectionMatrix).invert()}toJSON(t){const e=super.toJSON(t);return e.object.zoom=this.zoom,e.object.left=this.left,e.object.right=this.right,e.object.top=this.top,e.object.bottom=this.bottom,e.object.near=this.near,e.object.far=this.far,this.view!==null&&(e.object.view=Object.assign({},this.view)),e}}const Zr=4,ed=[.125,.215,.35,.446,.526,.582],ir=20,Oc=new mp,nd=new oe;let Fc=null,Vc=0,Bc=0;const tr=(1+Math.sqrt(5))/2,zr=1/tr,id=[new k(1,1,1),new k(-1,1,1),new k(1,1,-1),new k(-1,1,-1),new k(0,tr,zr),new k(0,tr,-zr),new k(zr,0,tr),new k(-zr,0,tr),new k(tr,zr,0),new k(-tr,zr,0)];class rd{constructor(t){this._renderer=t,this._pingPongRenderTarget=null,this._lodMax=0,this._cubeSize=0,this._lodPlanes=[],this._sizeLods=[],this._sigmas=[],this._blurMaterial=null,this._cubemapMaterial=null,this._equirectMaterial=null,this._compileMaterial(this._blurMaterial)}fromScene(t,e=0,n=.1,r=100){Fc=this._renderer.getRenderTarget(),Vc=this._renderer.getActiveCubeFace(),Bc=this._renderer.getActiveMipmapLevel(),this._setSize(256);const s=this._allocateTargets();return s.depthBuffer=!0,this._sceneToCubeUV(t,n,r,s),e>0&&this._blur(s,0,0,e),this._applyPMREM(s),this._cleanup(s),s}fromEquirectangular(t,e=null){return this._fromTexture(t,e)}fromCubemap(t,e=null){return this._fromTexture(t,e)}compileCubemapShader(){this._cubemapMaterial===null&&(this._cubemapMaterial=ad(),this._compileMaterial(this._cubemapMaterial))}compileEquirectangularShader(){this._equirectMaterial===null&&(this._equirectMaterial=od(),this._compileMaterial(this._equirectMaterial))}dispose(){this._dispose(),this._cubemapMaterial!==null&&this._cubemapMaterial.dispose(),this._equirectMaterial!==null&&this._equirectMaterial.dispose()}_setSize(t){this._lodMax=Math.floor(Math.log2(t)),this._cubeSize=Math.pow(2,this._lodMax)}_dispose(){this._blurMaterial!==null&&this._blurMaterial.dispose(),this._pingPongRenderTarget!==null&&this._pingPongRenderTarget.dispose();for(let t=0;t<this._lodPlanes.length;t++)this._lodPlanes[t].dispose()}_cleanup(t){this._renderer.setRenderTarget(Fc,Vc,Bc),t.scissorTest=!1,ta(t,0,0,t.width,t.height)}_fromTexture(t,e){t.mapping===rs||t.mapping===ss?this._setSize(t.image.length===0?16:t.image[0].width||t.image[0].image.width):this._setSize(t.image.width/4),Fc=this._renderer.getRenderTarget(),Vc=this._renderer.getActiveCubeFace(),Bc=this._renderer.getActiveMipmapLevel();const n=e||this._allocateTargets();return this._textureToCubeUV(t,n),this._applyPMREM(n),this._cleanup(n),n}_allocateTargets(){const t=3*Math.max(this._cubeSize,112),e=4*this._cubeSize,n={magFilter:Pn,minFilter:Pn,generateMipmaps:!1,type:eo,format:Wn,colorSpace:di,depthBuffer:!1},r=sd(t,e,n);if(this._pingPongRenderTarget===null||this._pingPongRenderTarget.width!==t||this._pingPongRenderTarget.height!==e){this._pingPongRenderTarget!==null&&this._dispose(),this._pingPongRenderTarget=sd(t,e,n);const{_lodMax:s}=this;({sizeLods:this._sizeLods,lodPlanes:this._lodPlanes,sigmas:this._sigmas}=Uy(s)),this._blurMaterial=Oy(s,t,e)}return r}_compileMaterial(t){const e=new Ie(this._lodPlanes[0],t);this._renderer.compile(e,Oc)}_sceneToCubeUV(t,e,n,r){const a=new Ln(90,1,e,n),c=[1,-1,1,1,1,1],h=[1,1,1,-1,-1,-1],u=this._renderer,d=u.autoClear,f=u.toneMapping;u.getClearColor(nd),u.toneMapping=Li,u.autoClear=!1;const p=new bn({name:"PMREM.Background",side:vn,depthWrite:!1,depthTest:!1}),y=new Ie(new _s,p);let E=!1;const _=t.background;_?_.isColor&&(p.color.copy(_),t.background=null,E=!0):(p.color.copy(nd),E=!0);for(let m=0;m<6;m++){const C=m%3;C===0?(a.up.set(0,c[m],0),a.lookAt(h[m],0,0)):C===1?(a.up.set(0,0,c[m]),a.lookAt(0,h[m],0)):(a.up.set(0,c[m],0),a.lookAt(0,0,h[m]));const w=this._cubeSize;ta(r,C*w,m>2?w:0,w,w),u.setRenderTarget(r),E&&u.render(y,a),u.render(t,a)}y.geometry.dispose(),y.material.dispose(),u.toneMapping=f,u.autoClear=d,t.background=_}_textureToCubeUV(t,e){const n=this._renderer,r=t.mapping===rs||t.mapping===ss;r?(this._cubemapMaterial===null&&(this._cubemapMaterial=ad()),this._cubemapMaterial.uniforms.flipEnvMap.value=t.isRenderTargetTexture===!1?-1:1):this._equirectMaterial===null&&(this._equirectMaterial=od());const s=r?this._cubemapMaterial:this._equirectMaterial,o=new Ie(this._lodPlanes[0],s),a=s.uniforms;a.envMap.value=t;const c=this._cubeSize;ta(e,0,0,3*c,2*c),n.setRenderTarget(e),n.render(o,Oc)}_applyPMREM(t){const e=this._renderer,n=e.autoClear;e.autoClear=!1;for(let r=1;r<this._lodPlanes.length;r++){const s=Math.sqrt(this._sigmas[r]*this._sigmas[r]-this._sigmas[r-1]*this._sigmas[r-1]),o=id[(r-1)%id.length];this._blur(t,r-1,r,s,o)}e.autoClear=n}_blur(t,e,n,r,s){const o=this._pingPongRenderTarget;this._halfBlur(t,o,e,n,r,"latitudinal",s),this._halfBlur(o,t,n,n,r,"longitudinal",s)}_halfBlur(t,e,n,r,s,o,a){const c=this._renderer,h=this._blurMaterial;o!=="latitudinal"&&o!=="longitudinal"&&console.error("blur direction must be either latitudinal or longitudinal!");const u=3,d=new Ie(this._lodPlanes[r],h),f=h.uniforms,p=this._sizeLods[n]-1,y=isFinite(s)?Math.PI/(2*p):2*Math.PI/(2*ir-1),E=s/y,_=isFinite(s)?1+Math.floor(u*E):ir;_>ir&&console.warn(`sigmaRadians, ${s}, is too large and will clip, as it requested ${_} samples when the maximum is set to ${ir}`);const m=[];let C=0;for(let T=0;T<ir;++T){const M=T/E,v=Math.exp(-M*M/2);m.push(v),T===0?C+=v:T<_&&(C+=2*v)}for(let T=0;T<m.length;T++)m[T]=m[T]/C;f.envMap.value=t.texture,f.samples.value=_,f.weights.value=m,f.latitudinal.value=o==="latitudinal",a&&(f.poleAxis.value=a);const{_lodMax:w}=this;f.dTheta.value=y,f.mipInt.value=w-n;const b=this._sizeLods[r],N=3*b*(r>w-Zr?r-w+Zr:0),U=4*(this._cubeSize-b);ta(e,N,U,3*b,2*b),c.setRenderTarget(e),c.render(d,Oc)}}function Uy(i){const t=[],e=[],n=[];let r=i;const s=i-Zr+1+ed.length;for(let o=0;o<s;o++){const a=Math.pow(2,r);e.push(a);let c=1/a;o>i-Zr?c=ed[o-i+Zr-1]:o===0&&(c=0),n.push(c);const h=1/(a-2),u=-h,d=1+h,f=[u,u,d,u,d,d,u,u,d,d,u,d],p=6,y=6,E=3,_=2,m=1,C=new Float32Array(E*y*p),w=new Float32Array(_*y*p),b=new Float32Array(m*y*p);for(let U=0;U<p;U++){const T=U%3*2/3-1,M=U>2?0:-1,v=[T,M,0,T+2/3,M,0,T+2/3,M+1,0,T,M,0,T+2/3,M+1,0,T,M+1,0];C.set(v,E*y*U),w.set(f,_*y*U);const S=[U,U,U,U,U,U];b.set(S,m*y*U)}const N=new yn;N.setAttribute("position",new xn(C,E)),N.setAttribute("uv",new xn(w,_)),N.setAttribute("faceIndex",new xn(b,m)),t.push(N),r>Zr&&r--}return{lodPlanes:t,sizeLods:e,sigmas:n}}function sd(i,t,e){const n=new hr(i,t,e);return n.texture.mapping=Ga,n.texture.name="PMREM.cubeUv",n.scissorTest=!0,n}function ta(i,t,e,n,r){i.viewport.set(t,e,n,r),i.scissor.set(t,e,n,r)}function Oy(i,t,e){const n=new Float32Array(ir),r=new k(0,1,0);return new fi({name:"SphericalGaussianBlur",defines:{n:ir,CUBEUV_TEXEL_WIDTH:1/t,CUBEUV_TEXEL_HEIGHT:1/e,CUBEUV_MAX_MIP:`${i}.0`},uniforms:{envMap:{value:null},samples:{value:1},weights:{value:n},latitudinal:{value:!1},dTheta:{value:0},mipInt:{value:0},poleAxis:{value:r}},vertexShader:Gl(),fragmentShader:`

			precision mediump float;
			precision mediump int;

			varying vec3 vOutputDirection;

			uniform sampler2D envMap;
			uniform int samples;
			uniform float weights[ n ];
			uniform bool latitudinal;
			uniform float dTheta;
			uniform float mipInt;
			uniform vec3 poleAxis;

			#define ENVMAP_TYPE_CUBE_UV
			#include <cube_uv_reflection_fragment>

			vec3 getSample( float theta, vec3 axis ) {

				float cosTheta = cos( theta );
				// Rodrigues' axis-angle rotation
				vec3 sampleDirection = vOutputDirection * cosTheta
					+ cross( axis, vOutputDirection ) * sin( theta )
					+ axis * dot( axis, vOutputDirection ) * ( 1.0 - cosTheta );

				return bilinearCubeUV( envMap, sampleDirection, mipInt );

			}

			void main() {

				vec3 axis = latitudinal ? poleAxis : cross( poleAxis, vOutputDirection );

				if ( all( equal( axis, vec3( 0.0 ) ) ) ) {

					axis = vec3( vOutputDirection.z, 0.0, - vOutputDirection.x );

				}

				axis = normalize( axis );

				gl_FragColor = vec4( 0.0, 0.0, 0.0, 1.0 );
				gl_FragColor.rgb += weights[ 0 ] * getSample( 0.0, axis );

				for ( int i = 1; i < n; i++ ) {

					if ( i >= samples ) {

						break;

					}

					float theta = dTheta * float( i );
					gl_FragColor.rgb += weights[ i ] * getSample( -1.0 * theta, axis );
					gl_FragColor.rgb += weights[ i ] * getSample( theta, axis );

				}

			}
		`,blending:Di,depthTest:!1,depthWrite:!1})}function od(){return new fi({name:"EquirectangularToCubeUV",uniforms:{envMap:{value:null}},vertexShader:Gl(),fragmentShader:`

			precision mediump float;
			precision mediump int;

			varying vec3 vOutputDirection;

			uniform sampler2D envMap;

			#include <common>

			void main() {

				vec3 outputDirection = normalize( vOutputDirection );
				vec2 uv = equirectUv( outputDirection );

				gl_FragColor = vec4( texture2D ( envMap, uv ).rgb, 1.0 );

			}
		`,blending:Di,depthTest:!1,depthWrite:!1})}function ad(){return new fi({name:"CubemapToCubeUV",uniforms:{envMap:{value:null},flipEnvMap:{value:-1}},vertexShader:Gl(),fragmentShader:`

			precision mediump float;
			precision mediump int;

			uniform float flipEnvMap;

			varying vec3 vOutputDirection;

			uniform samplerCube envMap;

			void main() {

				gl_FragColor = textureCube( envMap, vec3( flipEnvMap * vOutputDirection.x, vOutputDirection.yz ) );

			}
		`,blending:Di,depthTest:!1,depthWrite:!1})}function Gl(){return`

		precision mediump float;
		precision mediump int;

		attribute float faceIndex;

		varying vec3 vOutputDirection;

		// RH coordinate system; PMREM face-indexing convention
		vec3 getDirection( vec2 uv, float face ) {

			uv = 2.0 * uv - 1.0;

			vec3 direction = vec3( uv, 1.0 );

			if ( face == 0.0 ) {

				direction = direction.zyx; // ( 1, v, u ) pos x

			} else if ( face == 1.0 ) {

				direction = direction.xzy;
				direction.xz *= -1.0; // ( -u, 1, -v ) pos y

			} else if ( face == 2.0 ) {

				direction.x *= -1.0; // ( -u, v, 1 ) pos z

			} else if ( face == 3.0 ) {

				direction = direction.zyx;
				direction.xz *= -1.0; // ( -1, v, -u ) neg x

			} else if ( face == 4.0 ) {

				direction = direction.xzy;
				direction.xy *= -1.0; // ( -u, -1, v ) neg y

			} else if ( face == 5.0 ) {

				direction.z *= -1.0; // ( u, v, -1 ) neg z

			}

			return direction;

		}

		void main() {

			vOutputDirection = getDirection( uv, faceIndex );
			gl_Position = vec4( position, 1.0 );

		}
	`}function Fy(i){let t=new WeakMap,e=null;function n(a){if(a&&a.isTexture){const c=a.mapping,h=c===ol||c===al,u=c===rs||c===ss;if(h||u)if(a.isRenderTargetTexture&&a.needsPMREMUpdate===!0){a.needsPMREMUpdate=!1;let d=t.get(a);return e===null&&(e=new rd(i)),d=h?e.fromEquirectangular(a,d):e.fromCubemap(a,d),t.set(a,d),d.texture}else{if(t.has(a))return t.get(a).texture;{const d=a.image;if(h&&d&&d.height>0||u&&d&&r(d)){e===null&&(e=new rd(i));const f=h?e.fromEquirectangular(a):e.fromCubemap(a);return t.set(a,f),a.addEventListener("dispose",s),f.texture}else return null}}}return a}function r(a){let c=0;const h=6;for(let u=0;u<h;u++)a[u]!==void 0&&c++;return c===h}function s(a){const c=a.target;c.removeEventListener("dispose",s);const h=t.get(c);h!==void 0&&(t.delete(c),h.dispose())}function o(){t=new WeakMap,e!==null&&(e.dispose(),e=null)}return{get:n,dispose:o}}function Vy(i){const t={};function e(n){if(t[n]!==void 0)return t[n];let r;switch(n){case"WEBGL_depth_texture":r=i.getExtension("WEBGL_depth_texture")||i.getExtension("MOZ_WEBGL_depth_texture")||i.getExtension("WEBKIT_WEBGL_depth_texture");break;case"EXT_texture_filter_anisotropic":r=i.getExtension("EXT_texture_filter_anisotropic")||i.getExtension("MOZ_EXT_texture_filter_anisotropic")||i.getExtension("WEBKIT_EXT_texture_filter_anisotropic");break;case"WEBGL_compressed_texture_s3tc":r=i.getExtension("WEBGL_compressed_texture_s3tc")||i.getExtension("MOZ_WEBGL_compressed_texture_s3tc")||i.getExtension("WEBKIT_WEBGL_compressed_texture_s3tc");break;case"WEBGL_compressed_texture_pvrtc":r=i.getExtension("WEBGL_compressed_texture_pvrtc")||i.getExtension("WEBKIT_WEBGL_compressed_texture_pvrtc");break;default:r=i.getExtension(n)}return t[n]=r,r}return{has:function(n){return e(n)!==null},init:function(n){n.isWebGL2?(e("EXT_color_buffer_float"),e("WEBGL_clip_cull_distance")):(e("WEBGL_depth_texture"),e("OES_texture_float"),e("OES_texture_half_float"),e("OES_texture_half_float_linear"),e("OES_standard_derivatives"),e("OES_element_index_uint"),e("OES_vertex_array_object"),e("ANGLE_instanced_arrays")),e("OES_texture_float_linear"),e("EXT_color_buffer_half_float"),e("WEBGL_multisampled_render_to_texture")},get:function(n){const r=e(n);return r===null&&console.warn("THREE.WebGLRenderer: "+n+" extension not supported."),r}}}function By(i,t,e,n){const r={},s=new WeakMap;function o(d){const f=d.target;f.index!==null&&t.remove(f.index);for(const y in f.attributes)t.remove(f.attributes[y]);for(const y in f.morphAttributes){const E=f.morphAttributes[y];for(let _=0,m=E.length;_<m;_++)t.remove(E[_])}f.removeEventListener("dispose",o),delete r[f.id];const p=s.get(f);p&&(t.remove(p),s.delete(f)),n.releaseStatesOfGeometry(f),f.isInstancedBufferGeometry===!0&&delete f._maxInstanceCount,e.memory.geometries--}function a(d,f){return r[f.id]===!0||(f.addEventListener("dispose",o),r[f.id]=!0,e.memory.geometries++),f}function c(d){const f=d.attributes;for(const y in f)t.update(f[y],i.ARRAY_BUFFER);const p=d.morphAttributes;for(const y in p){const E=p[y];for(let _=0,m=E.length;_<m;_++)t.update(E[_],i.ARRAY_BUFFER)}}function h(d){const f=[],p=d.index,y=d.attributes.position;let E=0;if(p!==null){const C=p.array;E=p.version;for(let w=0,b=C.length;w<b;w+=3){const N=C[w+0],U=C[w+1],T=C[w+2];f.push(N,U,U,T,T,N)}}else if(y!==void 0){const C=y.array;E=y.version;for(let w=0,b=C.length/3-1;w<b;w+=3){const N=w+0,U=w+1,T=w+2;f.push(N,U,U,T,T,N)}}else return;const _=new(rp(f)?hp:lp)(f,1);_.version=E;const m=s.get(d);m&&t.remove(m),s.set(d,_)}function u(d){const f=s.get(d);if(f){const p=d.index;p!==null&&f.version<p.version&&h(d)}else h(d);return s.get(d)}return{get:a,update:c,getWireframeAttribute:u}}function ky(i,t,e,n){const r=n.isWebGL2;let s;function o(p){s=p}let a,c;function h(p){a=p.type,c=p.bytesPerElement}function u(p,y){i.drawElements(s,y,a,p*c),e.update(y,s,1)}function d(p,y,E){if(E===0)return;let _,m;if(r)_=i,m="drawElementsInstanced";else if(_=t.get("ANGLE_instanced_arrays"),m="drawElementsInstancedANGLE",_===null){console.error("THREE.WebGLIndexedBufferRenderer: using THREE.InstancedBufferGeometry but hardware does not support extension ANGLE_instanced_arrays.");return}_[m](s,y,a,p*c,E),e.update(y,s,E)}function f(p,y,E){if(E===0)return;const _=t.get("WEBGL_multi_draw");if(_===null)for(let m=0;m<E;m++)this.render(p[m]/c,y[m]);else{_.multiDrawElementsWEBGL(s,y,0,a,p,0,E);let m=0;for(let C=0;C<E;C++)m+=y[C];e.update(m,s,1)}}this.setMode=o,this.setIndex=h,this.render=u,this.renderInstances=d,this.renderMultiDraw=f}function Hy(i){const t={geometries:0,textures:0},e={frame:0,calls:0,triangles:0,points:0,lines:0};function n(s,o,a){switch(e.calls++,o){case i.TRIANGLES:e.triangles+=a*(s/3);break;case i.LINES:e.lines+=a*(s/2);break;case i.LINE_STRIP:e.lines+=a*(s-1);break;case i.LINE_LOOP:e.lines+=a*s;break;case i.POINTS:e.points+=a*s;break;default:console.error("THREE.WebGLInfo: Unknown draw mode:",o);break}}function r(){e.calls=0,e.triangles=0,e.points=0,e.lines=0}return{memory:t,render:e,programs:null,autoReset:!0,reset:r,update:n}}function zy(i,t){return i[0]-t[0]}function Gy(i,t){return Math.abs(t[1])-Math.abs(i[1])}function Wy(i,t,e){const n={},r=new Float32Array(8),s=new WeakMap,o=new Xe,a=[];for(let h=0;h<8;h++)a[h]=[h,0];function c(h,u,d){const f=h.morphTargetInfluences;if(t.isWebGL2===!0){const y=u.morphAttributes.position||u.morphAttributes.normal||u.morphAttributes.color,E=y!==void 0?y.length:0;let _=s.get(u);if(_===void 0||_.count!==E){let G=function(){R.dispose(),s.delete(u),u.removeEventListener("dispose",G)};var p=G;_!==void 0&&_.texture.dispose();const w=u.morphAttributes.position!==void 0,b=u.morphAttributes.normal!==void 0,N=u.morphAttributes.color!==void 0,U=u.morphAttributes.position||[],T=u.morphAttributes.normal||[],M=u.morphAttributes.color||[];let v=0;w===!0&&(v=1),b===!0&&(v=2),N===!0&&(v=3);let S=u.attributes.position.count*v,P=1;S>t.maxTextureSize&&(P=Math.ceil(S/t.maxTextureSize),S=t.maxTextureSize);const L=new Float32Array(S*P*4*E),R=new ap(L,S,P,E);R.type=Ci,R.needsUpdate=!0;const V=v*4;for(let K=0;K<E;K++){const Z=U[K],J=T[K],et=M[K],tt=S*P*4*K;for(let pt=0;pt<Z.count;pt++){const ht=pt*V;w===!0&&(o.fromBufferAttribute(Z,pt),L[tt+ht+0]=o.x,L[tt+ht+1]=o.y,L[tt+ht+2]=o.z,L[tt+ht+3]=0),b===!0&&(o.fromBufferAttribute(J,pt),L[tt+ht+4]=o.x,L[tt+ht+5]=o.y,L[tt+ht+6]=o.z,L[tt+ht+7]=0),N===!0&&(o.fromBufferAttribute(et,pt),L[tt+ht+8]=o.x,L[tt+ht+9]=o.y,L[tt+ht+10]=o.z,L[tt+ht+11]=et.itemSize===4?o.w:1)}}_={count:E,texture:R,size:new Vt(S,P)},s.set(u,_),u.addEventListener("dispose",G)}let m=0;for(let w=0;w<f.length;w++)m+=f[w];const C=u.morphTargetsRelative?1:1-m;d.getUniforms().setValue(i,"morphTargetBaseInfluence",C),d.getUniforms().setValue(i,"morphTargetInfluences",f),d.getUniforms().setValue(i,"morphTargetsTexture",_.texture,e),d.getUniforms().setValue(i,"morphTargetsTextureSize",_.size)}else{const y=f===void 0?0:f.length;let E=n[u.id];if(E===void 0||E.length!==y){E=[];for(let b=0;b<y;b++)E[b]=[b,0];n[u.id]=E}for(let b=0;b<y;b++){const N=E[b];N[0]=b,N[1]=f[b]}E.sort(Gy);for(let b=0;b<8;b++)b<y&&E[b][1]?(a[b][0]=E[b][0],a[b][1]=E[b][1]):(a[b][0]=Number.MAX_SAFE_INTEGER,a[b][1]=0);a.sort(zy);const _=u.morphAttributes.position,m=u.morphAttributes.normal;let C=0;for(let b=0;b<8;b++){const N=a[b],U=N[0],T=N[1];U!==Number.MAX_SAFE_INTEGER&&T?(_&&u.getAttribute("morphTarget"+b)!==_[U]&&u.setAttribute("morphTarget"+b,_[U]),m&&u.getAttribute("morphNormal"+b)!==m[U]&&u.setAttribute("morphNormal"+b,m[U]),r[b]=T,C+=T):(_&&u.hasAttribute("morphTarget"+b)===!0&&u.deleteAttribute("morphTarget"+b),m&&u.hasAttribute("morphNormal"+b)===!0&&u.deleteAttribute("morphNormal"+b),r[b]=0)}const w=u.morphTargetsRelative?1:1-C;d.getUniforms().setValue(i,"morphTargetBaseInfluence",w),d.getUniforms().setValue(i,"morphTargetInfluences",r)}}return{update:c}}function qy(i,t,e,n){let r=new WeakMap;function s(c){const h=n.render.frame,u=c.geometry,d=t.get(c,u);if(r.get(d)!==h&&(t.update(d),r.set(d,h)),c.isInstancedMesh&&(c.hasEventListener("dispose",a)===!1&&c.addEventListener("dispose",a),r.get(c)!==h&&(e.update(c.instanceMatrix,i.ARRAY_BUFFER),c.instanceColor!==null&&e.update(c.instanceColor,i.ARRAY_BUFFER),r.set(c,h))),c.isSkinnedMesh){const f=c.skeleton;r.get(f)!==h&&(f.update(),r.set(f,h))}return d}function o(){r=new WeakMap}function a(c){const h=c.target;h.removeEventListener("dispose",a),e.remove(h.instanceMatrix),h.instanceColor!==null&&e.remove(h.instanceColor)}return{update:s,dispose:o}}class gp extends En{constructor(t,e,n,r,s,o,a,c,h,u){if(u=u!==void 0?u:ar,u!==ar&&u!==os)throw new Error("DepthTexture format must be either THREE.DepthFormat or THREE.DepthStencilFormat");n===void 0&&u===ar&&(n=Ri),n===void 0&&u===os&&(n=or),super(null,r,s,o,a,c,u,n,h),this.isDepthTexture=!0,this.image={width:t,height:e},this.magFilter=a!==void 0?a:pn,this.minFilter=c!==void 0?c:pn,this.flipY=!1,this.generateMipmaps=!1,this.compareFunction=null}copy(t){return super.copy(t),this.compareFunction=t.compareFunction,this}toJSON(t){const e=super.toJSON(t);return this.compareFunction!==null&&(e.compareFunction=this.compareFunction),e}}const _p=new En,vp=new gp(1,1);vp.compareFunction=ip;const yp=new ap,Ep=new C_,xp=new fp,cd=[],ld=[],hd=new Float32Array(16),ud=new Float32Array(9),dd=new Float32Array(4);function vs(i,t,e){const n=i[0];if(n<=0||n>0)return i;const r=t*e;let s=cd[r];if(s===void 0&&(s=new Float32Array(r),cd[r]=s),t!==0){n.toArray(s,0);for(let o=1,a=0;o!==t;++o)a+=e,i[o].toArray(s,a)}return s}function ze(i,t){if(i.length!==t.length)return!1;for(let e=0,n=i.length;e<n;e++)if(i[e]!==t[e])return!1;return!0}function Ge(i,t){for(let e=0,n=t.length;e<n;e++)i[e]=t[e]}function Xa(i,t){let e=ld[t];e===void 0&&(e=new Int32Array(t),ld[t]=e);for(let n=0;n!==t;++n)e[n]=i.allocateTextureUnit();return e}function Xy(i,t){const e=this.cache;e[0]!==t&&(i.uniform1f(this.addr,t),e[0]=t)}function jy(i,t){const e=this.cache;if(t.x!==void 0)(e[0]!==t.x||e[1]!==t.y)&&(i.uniform2f(this.addr,t.x,t.y),e[0]=t.x,e[1]=t.y);else{if(ze(e,t))return;i.uniform2fv(this.addr,t),Ge(e,t)}}function Yy(i,t){const e=this.cache;if(t.x!==void 0)(e[0]!==t.x||e[1]!==t.y||e[2]!==t.z)&&(i.uniform3f(this.addr,t.x,t.y,t.z),e[0]=t.x,e[1]=t.y,e[2]=t.z);else if(t.r!==void 0)(e[0]!==t.r||e[1]!==t.g||e[2]!==t.b)&&(i.uniform3f(this.addr,t.r,t.g,t.b),e[0]=t.r,e[1]=t.g,e[2]=t.b);else{if(ze(e,t))return;i.uniform3fv(this.addr,t),Ge(e,t)}}function $y(i,t){const e=this.cache;if(t.x!==void 0)(e[0]!==t.x||e[1]!==t.y||e[2]!==t.z||e[3]!==t.w)&&(i.uniform4f(this.addr,t.x,t.y,t.z,t.w),e[0]=t.x,e[1]=t.y,e[2]=t.z,e[3]=t.w);else{if(ze(e,t))return;i.uniform4fv(this.addr,t),Ge(e,t)}}function Ky(i,t){const e=this.cache,n=t.elements;if(n===void 0){if(ze(e,t))return;i.uniformMatrix2fv(this.addr,!1,t),Ge(e,t)}else{if(ze(e,n))return;dd.set(n),i.uniformMatrix2fv(this.addr,!1,dd),Ge(e,n)}}function Qy(i,t){const e=this.cache,n=t.elements;if(n===void 0){if(ze(e,t))return;i.uniformMatrix3fv(this.addr,!1,t),Ge(e,t)}else{if(ze(e,n))return;ud.set(n),i.uniformMatrix3fv(this.addr,!1,ud),Ge(e,n)}}function Zy(i,t){const e=this.cache,n=t.elements;if(n===void 0){if(ze(e,t))return;i.uniformMatrix4fv(this.addr,!1,t),Ge(e,t)}else{if(ze(e,n))return;hd.set(n),i.uniformMatrix4fv(this.addr,!1,hd),Ge(e,n)}}function Jy(i,t){const e=this.cache;e[0]!==t&&(i.uniform1i(this.addr,t),e[0]=t)}function tE(i,t){const e=this.cache;if(t.x!==void 0)(e[0]!==t.x||e[1]!==t.y)&&(i.uniform2i(this.addr,t.x,t.y),e[0]=t.x,e[1]=t.y);else{if(ze(e,t))return;i.uniform2iv(this.addr,t),Ge(e,t)}}function eE(i,t){const e=this.cache;if(t.x!==void 0)(e[0]!==t.x||e[1]!==t.y||e[2]!==t.z)&&(i.uniform3i(this.addr,t.x,t.y,t.z),e[0]=t.x,e[1]=t.y,e[2]=t.z);else{if(ze(e,t))return;i.uniform3iv(this.addr,t),Ge(e,t)}}function nE(i,t){const e=this.cache;if(t.x!==void 0)(e[0]!==t.x||e[1]!==t.y||e[2]!==t.z||e[3]!==t.w)&&(i.uniform4i(this.addr,t.x,t.y,t.z,t.w),e[0]=t.x,e[1]=t.y,e[2]=t.z,e[3]=t.w);else{if(ze(e,t))return;i.uniform4iv(this.addr,t),Ge(e,t)}}function iE(i,t){const e=this.cache;e[0]!==t&&(i.uniform1ui(this.addr,t),e[0]=t)}function rE(i,t){const e=this.cache;if(t.x!==void 0)(e[0]!==t.x||e[1]!==t.y)&&(i.uniform2ui(this.addr,t.x,t.y),e[0]=t.x,e[1]=t.y);else{if(ze(e,t))return;i.uniform2uiv(this.addr,t),Ge(e,t)}}function sE(i,t){const e=this.cache;if(t.x!==void 0)(e[0]!==t.x||e[1]!==t.y||e[2]!==t.z)&&(i.uniform3ui(this.addr,t.x,t.y,t.z),e[0]=t.x,e[1]=t.y,e[2]=t.z);else{if(ze(e,t))return;i.uniform3uiv(this.addr,t),Ge(e,t)}}function oE(i,t){const e=this.cache;if(t.x!==void 0)(e[0]!==t.x||e[1]!==t.y||e[2]!==t.z||e[3]!==t.w)&&(i.uniform4ui(this.addr,t.x,t.y,t.z,t.w),e[0]=t.x,e[1]=t.y,e[2]=t.z,e[3]=t.w);else{if(ze(e,t))return;i.uniform4uiv(this.addr,t),Ge(e,t)}}function aE(i,t,e){const n=this.cache,r=e.allocateTextureUnit();n[0]!==r&&(i.uniform1i(this.addr,r),n[0]=r);const s=this.type===i.SAMPLER_2D_SHADOW?vp:_p;e.setTexture2D(t||s,r)}function cE(i,t,e){const n=this.cache,r=e.allocateTextureUnit();n[0]!==r&&(i.uniform1i(this.addr,r),n[0]=r),e.setTexture3D(t||Ep,r)}function lE(i,t,e){const n=this.cache,r=e.allocateTextureUnit();n[0]!==r&&(i.uniform1i(this.addr,r),n[0]=r),e.setTextureCube(t||xp,r)}function hE(i,t,e){const n=this.cache,r=e.allocateTextureUnit();n[0]!==r&&(i.uniform1i(this.addr,r),n[0]=r),e.setTexture2DArray(t||yp,r)}function uE(i){switch(i){case 5126:return Xy;case 35664:return jy;case 35665:return Yy;case 35666:return $y;case 35674:return Ky;case 35675:return Qy;case 35676:return Zy;case 5124:case 35670:return Jy;case 35667:case 35671:return tE;case 35668:case 35672:return eE;case 35669:case 35673:return nE;case 5125:return iE;case 36294:return rE;case 36295:return sE;case 36296:return oE;case 35678:case 36198:case 36298:case 36306:case 35682:return aE;case 35679:case 36299:case 36307:return cE;case 35680:case 36300:case 36308:case 36293:return lE;case 36289:case 36303:case 36311:case 36292:return hE}}function dE(i,t){i.uniform1fv(this.addr,t)}function fE(i,t){const e=vs(t,this.size,2);i.uniform2fv(this.addr,e)}function pE(i,t){const e=vs(t,this.size,3);i.uniform3fv(this.addr,e)}function mE(i,t){const e=vs(t,this.size,4);i.uniform4fv(this.addr,e)}function gE(i,t){const e=vs(t,this.size,4);i.uniformMatrix2fv(this.addr,!1,e)}function _E(i,t){const e=vs(t,this.size,9);i.uniformMatrix3fv(this.addr,!1,e)}function vE(i,t){const e=vs(t,this.size,16);i.uniformMatrix4fv(this.addr,!1,e)}function yE(i,t){i.uniform1iv(this.addr,t)}function EE(i,t){i.uniform2iv(this.addr,t)}function xE(i,t){i.uniform3iv(this.addr,t)}function SE(i,t){i.uniform4iv(this.addr,t)}function TE(i,t){i.uniform1uiv(this.addr,t)}function ME(i,t){i.uniform2uiv(this.addr,t)}function wE(i,t){i.uniform3uiv(this.addr,t)}function bE(i,t){i.uniform4uiv(this.addr,t)}function AE(i,t,e){const n=this.cache,r=t.length,s=Xa(e,r);ze(n,s)||(i.uniform1iv(this.addr,s),Ge(n,s));for(let o=0;o!==r;++o)e.setTexture2D(t[o]||_p,s[o])}function RE(i,t,e){const n=this.cache,r=t.length,s=Xa(e,r);ze(n,s)||(i.uniform1iv(this.addr,s),Ge(n,s));for(let o=0;o!==r;++o)e.setTexture3D(t[o]||Ep,s[o])}function CE(i,t,e){const n=this.cache,r=t.length,s=Xa(e,r);ze(n,s)||(i.uniform1iv(this.addr,s),Ge(n,s));for(let o=0;o!==r;++o)e.setTextureCube(t[o]||xp,s[o])}function IE(i,t,e){const n=this.cache,r=t.length,s=Xa(e,r);ze(n,s)||(i.uniform1iv(this.addr,s),Ge(n,s));for(let o=0;o!==r;++o)e.setTexture2DArray(t[o]||yp,s[o])}function PE(i){switch(i){case 5126:return dE;case 35664:return fE;case 35665:return pE;case 35666:return mE;case 35674:return gE;case 35675:return _E;case 35676:return vE;case 5124:case 35670:return yE;case 35667:case 35671:return EE;case 35668:case 35672:return xE;case 35669:case 35673:return SE;case 5125:return TE;case 36294:return ME;case 36295:return wE;case 36296:return bE;case 35678:case 36198:case 36298:case 36306:case 35682:return AE;case 35679:case 36299:case 36307:return RE;case 35680:case 36300:case 36308:case 36293:return CE;case 36289:case 36303:case 36311:case 36292:return IE}}class DE{constructor(t,e,n){this.id=t,this.addr=n,this.cache=[],this.type=e.type,this.setValue=uE(e.type)}}class LE{constructor(t,e,n){this.id=t,this.addr=n,this.cache=[],this.type=e.type,this.size=e.size,this.setValue=PE(e.type)}}class NE{constructor(t){this.id=t,this.seq=[],this.map={}}setValue(t,e,n){const r=this.seq;for(let s=0,o=r.length;s!==o;++s){const a=r[s];a.setValue(t,e[a.id],n)}}}const kc=/(\w+)(\])?(\[|\.)?/g;function fd(i,t){i.seq.push(t),i.map[t.id]=t}function UE(i,t,e){const n=i.name,r=n.length;for(kc.lastIndex=0;;){const s=kc.exec(n),o=kc.lastIndex;let a=s[1];const c=s[2]==="]",h=s[3];if(c&&(a=a|0),h===void 0||h==="["&&o+2===r){fd(e,h===void 0?new DE(a,i,t):new LE(a,i,t));break}else{let d=e.map[a];d===void 0&&(d=new NE(a),fd(e,d)),e=d}}}class pa{constructor(t,e){this.seq=[],this.map={};const n=t.getProgramParameter(e,t.ACTIVE_UNIFORMS);for(let r=0;r<n;++r){const s=t.getActiveUniform(e,r),o=t.getUniformLocation(e,s.name);UE(s,o,this)}}setValue(t,e,n,r){const s=this.map[e];s!==void 0&&s.setValue(t,n,r)}setOptional(t,e,n){const r=e[n];r!==void 0&&this.setValue(t,n,r)}static upload(t,e,n,r){for(let s=0,o=e.length;s!==o;++s){const a=e[s],c=n[a.id];c.needsUpdate!==!1&&a.setValue(t,c.value,r)}}static seqWithValue(t,e){const n=[];for(let r=0,s=t.length;r!==s;++r){const o=t[r];o.id in e&&n.push(o)}return n}}function pd(i,t,e){const n=i.createShader(t);return i.shaderSource(n,e),i.compileShader(n),n}const OE=37297;let FE=0;function VE(i,t){const e=i.split(`
`),n=[],r=Math.max(t-6,0),s=Math.min(t+6,e.length);for(let o=r;o<s;o++){const a=o+1;n.push(`${a===t?">":" "} ${a}: ${e[o]}`)}return n.join(`
`)}function BE(i){const t=ve.getPrimaries(ve.workingColorSpace),e=ve.getPrimaries(i);let n;switch(t===e?n="":t===Ma&&e===Ta?n="LinearDisplayP3ToLinearSRGB":t===Ta&&e===Ma&&(n="LinearSRGBToLinearDisplayP3"),i){case di:case Wa:return[n,"LinearTransferOETF"];case Ze:case Vl:return[n,"sRGBTransferOETF"];default:return console.warn("THREE.WebGLProgram: Unsupported color space:",i),[n,"LinearTransferOETF"]}}function md(i,t,e){const n=i.getShaderParameter(t,i.COMPILE_STATUS),r=i.getShaderInfoLog(t).trim();if(n&&r==="")return"";const s=/ERROR: 0:(\d+)/.exec(r);if(s){const o=parseInt(s[1]);return e.toUpperCase()+`

`+r+`

`+VE(i.getShaderSource(t),o)}else return r}function kE(i,t){const e=BE(t);return`vec4 ${i}( vec4 value ) { return ${e[0]}( ${e[1]}( value ) ); }`}function HE(i,t){let e;switch(t){case Zg:e="Linear";break;case Jg:e="Reinhard";break;case t_:e="OptimizedCineon";break;case Xf:e="ACESFilmic";break;case n_:e="AgX";break;case e_:e="Custom";break;default:console.warn("THREE.WebGLProgram: Unsupported toneMapping:",t),e="Linear"}return"vec3 "+i+"( vec3 color ) { return "+e+"ToneMapping( color ); }"}function zE(i){return[i.extensionDerivatives||i.envMapCubeUVHeight||i.bumpMap||i.normalMapTangentSpace||i.clearcoatNormalMap||i.flatShading||i.shaderID==="physical"?"#extension GL_OES_standard_derivatives : enable":"",(i.extensionFragDepth||i.logarithmicDepthBuffer)&&i.rendererExtensionFragDepth?"#extension GL_EXT_frag_depth : enable":"",i.extensionDrawBuffers&&i.rendererExtensionDrawBuffers?"#extension GL_EXT_draw_buffers : require":"",(i.extensionShaderTextureLOD||i.envMap||i.transmission)&&i.rendererExtensionShaderTextureLod?"#extension GL_EXT_shader_texture_lod : enable":""].filter(Jr).join(`
`)}function GE(i){return[i.extensionClipCullDistance?"#extension GL_ANGLE_clip_cull_distance : require":""].filter(Jr).join(`
`)}function WE(i){const t=[];for(const e in i){const n=i[e];n!==!1&&t.push("#define "+e+" "+n)}return t.join(`
`)}function qE(i,t){const e={},n=i.getProgramParameter(t,i.ACTIVE_ATTRIBUTES);for(let r=0;r<n;r++){const s=i.getActiveAttrib(t,r),o=s.name;let a=1;s.type===i.FLOAT_MAT2&&(a=2),s.type===i.FLOAT_MAT3&&(a=3),s.type===i.FLOAT_MAT4&&(a=4),e[o]={type:s.type,location:i.getAttribLocation(t,o),locationSize:a}}return e}function Jr(i){return i!==""}function gd(i,t){const e=t.numSpotLightShadows+t.numSpotLightMaps-t.numSpotLightShadowsWithMaps;return i.replace(/NUM_DIR_LIGHTS/g,t.numDirLights).replace(/NUM_SPOT_LIGHTS/g,t.numSpotLights).replace(/NUM_SPOT_LIGHT_MAPS/g,t.numSpotLightMaps).replace(/NUM_SPOT_LIGHT_COORDS/g,e).replace(/NUM_RECT_AREA_LIGHTS/g,t.numRectAreaLights).replace(/NUM_POINT_LIGHTS/g,t.numPointLights).replace(/NUM_HEMI_LIGHTS/g,t.numHemiLights).replace(/NUM_DIR_LIGHT_SHADOWS/g,t.numDirLightShadows).replace(/NUM_SPOT_LIGHT_SHADOWS_WITH_MAPS/g,t.numSpotLightShadowsWithMaps).replace(/NUM_SPOT_LIGHT_SHADOWS/g,t.numSpotLightShadows).replace(/NUM_POINT_LIGHT_SHADOWS/g,t.numPointLightShadows)}function _d(i,t){return i.replace(/NUM_CLIPPING_PLANES/g,t.numClippingPlanes).replace(/UNION_CLIPPING_PLANES/g,t.numClippingPlanes-t.numClipIntersection)}const XE=/^[ \t]*#include +<([\w\d./]+)>/gm;function pl(i){return i.replace(XE,YE)}const jE=new Map([["encodings_fragment","colorspace_fragment"],["encodings_pars_fragment","colorspace_pars_fragment"],["output_fragment","opaque_fragment"]]);function YE(i,t){let e=re[t];if(e===void 0){const n=jE.get(t);if(n!==void 0)e=re[n],console.warn('THREE.WebGLRenderer: Shader chunk "%s" has been deprecated. Use "%s" instead.',t,n);else throw new Error("Can not resolve #include <"+t+">")}return pl(e)}const $E=/#pragma unroll_loop_start\s+for\s*\(\s*int\s+i\s*=\s*(\d+)\s*;\s*i\s*<\s*(\d+)\s*;\s*i\s*\+\+\s*\)\s*{([\s\S]+?)}\s+#pragma unroll_loop_end/g;function vd(i){return i.replace($E,KE)}function KE(i,t,e,n){let r="";for(let s=parseInt(t);s<parseInt(e);s++)r+=n.replace(/\[\s*i\s*\]/g,"[ "+s+" ]").replace(/UNROLLED_LOOP_INDEX/g,s);return r}function yd(i){let t="precision "+i.precision+` float;
precision `+i.precision+" int;";return i.precision==="highp"?t+=`
#define HIGH_PRECISION`:i.precision==="mediump"?t+=`
#define MEDIUM_PRECISION`:i.precision==="lowp"&&(t+=`
#define LOW_PRECISION`),t}function QE(i){let t="SHADOWMAP_TYPE_BASIC";return i.shadowMapType===Wf?t="SHADOWMAP_TYPE_PCF":i.shadowMapType===bg?t="SHADOWMAP_TYPE_PCF_SOFT":i.shadowMapType===oi&&(t="SHADOWMAP_TYPE_VSM"),t}function ZE(i){let t="ENVMAP_TYPE_CUBE";if(i.envMap)switch(i.envMapMode){case rs:case ss:t="ENVMAP_TYPE_CUBE";break;case Ga:t="ENVMAP_TYPE_CUBE_UV";break}return t}function JE(i){let t="ENVMAP_MODE_REFLECTION";if(i.envMap)switch(i.envMapMode){case ss:t="ENVMAP_MODE_REFRACTION";break}return t}function tx(i){let t="ENVMAP_BLENDING_NONE";if(i.envMap)switch(i.combine){case qf:t="ENVMAP_BLENDING_MULTIPLY";break;case Kg:t="ENVMAP_BLENDING_MIX";break;case Qg:t="ENVMAP_BLENDING_ADD";break}return t}function ex(i){const t=i.envMapCubeUVHeight;if(t===null)return null;const e=Math.log2(t)-2,n=1/t;return{texelWidth:1/(3*Math.max(Math.pow(2,e),7*16)),texelHeight:n,maxMip:e}}function nx(i,t,e,n){const r=i.getContext(),s=e.defines;let o=e.vertexShader,a=e.fragmentShader;const c=QE(e),h=ZE(e),u=JE(e),d=tx(e),f=ex(e),p=e.isWebGL2?"":zE(e),y=GE(e),E=WE(s),_=r.createProgram();let m,C,w=e.glslVersion?"#version "+e.glslVersion+`
`:"";e.isRawShaderMaterial?(m=["#define SHADER_TYPE "+e.shaderType,"#define SHADER_NAME "+e.shaderName,E].filter(Jr).join(`
`),m.length>0&&(m+=`
`),C=[p,"#define SHADER_TYPE "+e.shaderType,"#define SHADER_NAME "+e.shaderName,E].filter(Jr).join(`
`),C.length>0&&(C+=`
`)):(m=[yd(e),"#define SHADER_TYPE "+e.shaderType,"#define SHADER_NAME "+e.shaderName,E,e.extensionClipCullDistance?"#define USE_CLIP_DISTANCE":"",e.batching?"#define USE_BATCHING":"",e.instancing?"#define USE_INSTANCING":"",e.instancingColor?"#define USE_INSTANCING_COLOR":"",e.useFog&&e.fog?"#define USE_FOG":"",e.useFog&&e.fogExp2?"#define FOG_EXP2":"",e.map?"#define USE_MAP":"",e.envMap?"#define USE_ENVMAP":"",e.envMap?"#define "+u:"",e.lightMap?"#define USE_LIGHTMAP":"",e.aoMap?"#define USE_AOMAP":"",e.bumpMap?"#define USE_BUMPMAP":"",e.normalMap?"#define USE_NORMALMAP":"",e.normalMapObjectSpace?"#define USE_NORMALMAP_OBJECTSPACE":"",e.normalMapTangentSpace?"#define USE_NORMALMAP_TANGENTSPACE":"",e.displacementMap?"#define USE_DISPLACEMENTMAP":"",e.emissiveMap?"#define USE_EMISSIVEMAP":"",e.anisotropy?"#define USE_ANISOTROPY":"",e.anisotropyMap?"#define USE_ANISOTROPYMAP":"",e.clearcoatMap?"#define USE_CLEARCOATMAP":"",e.clearcoatRoughnessMap?"#define USE_CLEARCOAT_ROUGHNESSMAP":"",e.clearcoatNormalMap?"#define USE_CLEARCOAT_NORMALMAP":"",e.iridescenceMap?"#define USE_IRIDESCENCEMAP":"",e.iridescenceThicknessMap?"#define USE_IRIDESCENCE_THICKNESSMAP":"",e.specularMap?"#define USE_SPECULARMAP":"",e.specularColorMap?"#define USE_SPECULAR_COLORMAP":"",e.specularIntensityMap?"#define USE_SPECULAR_INTENSITYMAP":"",e.roughnessMap?"#define USE_ROUGHNESSMAP":"",e.metalnessMap?"#define USE_METALNESSMAP":"",e.alphaMap?"#define USE_ALPHAMAP":"",e.alphaHash?"#define USE_ALPHAHASH":"",e.transmission?"#define USE_TRANSMISSION":"",e.transmissionMap?"#define USE_TRANSMISSIONMAP":"",e.thicknessMap?"#define USE_THICKNESSMAP":"",e.sheenColorMap?"#define USE_SHEEN_COLORMAP":"",e.sheenRoughnessMap?"#define USE_SHEEN_ROUGHNESSMAP":"",e.mapUv?"#define MAP_UV "+e.mapUv:"",e.alphaMapUv?"#define ALPHAMAP_UV "+e.alphaMapUv:"",e.lightMapUv?"#define LIGHTMAP_UV "+e.lightMapUv:"",e.aoMapUv?"#define AOMAP_UV "+e.aoMapUv:"",e.emissiveMapUv?"#define EMISSIVEMAP_UV "+e.emissiveMapUv:"",e.bumpMapUv?"#define BUMPMAP_UV "+e.bumpMapUv:"",e.normalMapUv?"#define NORMALMAP_UV "+e.normalMapUv:"",e.displacementMapUv?"#define DISPLACEMENTMAP_UV "+e.displacementMapUv:"",e.metalnessMapUv?"#define METALNESSMAP_UV "+e.metalnessMapUv:"",e.roughnessMapUv?"#define ROUGHNESSMAP_UV "+e.roughnessMapUv:"",e.anisotropyMapUv?"#define ANISOTROPYMAP_UV "+e.anisotropyMapUv:"",e.clearcoatMapUv?"#define CLEARCOATMAP_UV "+e.clearcoatMapUv:"",e.clearcoatNormalMapUv?"#define CLEARCOAT_NORMALMAP_UV "+e.clearcoatNormalMapUv:"",e.clearcoatRoughnessMapUv?"#define CLEARCOAT_ROUGHNESSMAP_UV "+e.clearcoatRoughnessMapUv:"",e.iridescenceMapUv?"#define IRIDESCENCEMAP_UV "+e.iridescenceMapUv:"",e.iridescenceThicknessMapUv?"#define IRIDESCENCE_THICKNESSMAP_UV "+e.iridescenceThicknessMapUv:"",e.sheenColorMapUv?"#define SHEEN_COLORMAP_UV "+e.sheenColorMapUv:"",e.sheenRoughnessMapUv?"#define SHEEN_ROUGHNESSMAP_UV "+e.sheenRoughnessMapUv:"",e.specularMapUv?"#define SPECULARMAP_UV "+e.specularMapUv:"",e.specularColorMapUv?"#define SPECULAR_COLORMAP_UV "+e.specularColorMapUv:"",e.specularIntensityMapUv?"#define SPECULAR_INTENSITYMAP_UV "+e.specularIntensityMapUv:"",e.transmissionMapUv?"#define TRANSMISSIONMAP_UV "+e.transmissionMapUv:"",e.thicknessMapUv?"#define THICKNESSMAP_UV "+e.thicknessMapUv:"",e.vertexTangents&&e.flatShading===!1?"#define USE_TANGENT":"",e.vertexColors?"#define USE_COLOR":"",e.vertexAlphas?"#define USE_COLOR_ALPHA":"",e.vertexUv1s?"#define USE_UV1":"",e.vertexUv2s?"#define USE_UV2":"",e.vertexUv3s?"#define USE_UV3":"",e.pointsUvs?"#define USE_POINTS_UV":"",e.flatShading?"#define FLAT_SHADED":"",e.skinning?"#define USE_SKINNING":"",e.morphTargets?"#define USE_MORPHTARGETS":"",e.morphNormals&&e.flatShading===!1?"#define USE_MORPHNORMALS":"",e.morphColors&&e.isWebGL2?"#define USE_MORPHCOLORS":"",e.morphTargetsCount>0&&e.isWebGL2?"#define MORPHTARGETS_TEXTURE":"",e.morphTargetsCount>0&&e.isWebGL2?"#define MORPHTARGETS_TEXTURE_STRIDE "+e.morphTextureStride:"",e.morphTargetsCount>0&&e.isWebGL2?"#define MORPHTARGETS_COUNT "+e.morphTargetsCount:"",e.doubleSided?"#define DOUBLE_SIDED":"",e.flipSided?"#define FLIP_SIDED":"",e.shadowMapEnabled?"#define USE_SHADOWMAP":"",e.shadowMapEnabled?"#define "+c:"",e.sizeAttenuation?"#define USE_SIZEATTENUATION":"",e.numLightProbes>0?"#define USE_LIGHT_PROBES":"",e.useLegacyLights?"#define LEGACY_LIGHTS":"",e.logarithmicDepthBuffer?"#define USE_LOGDEPTHBUF":"",e.logarithmicDepthBuffer&&e.rendererExtensionFragDepth?"#define USE_LOGDEPTHBUF_EXT":"","uniform mat4 modelMatrix;","uniform mat4 modelViewMatrix;","uniform mat4 projectionMatrix;","uniform mat4 viewMatrix;","uniform mat3 normalMatrix;","uniform vec3 cameraPosition;","uniform bool isOrthographic;","#ifdef USE_INSTANCING","	attribute mat4 instanceMatrix;","#endif","#ifdef USE_INSTANCING_COLOR","	attribute vec3 instanceColor;","#endif","attribute vec3 position;","attribute vec3 normal;","attribute vec2 uv;","#ifdef USE_UV1","	attribute vec2 uv1;","#endif","#ifdef USE_UV2","	attribute vec2 uv2;","#endif","#ifdef USE_UV3","	attribute vec2 uv3;","#endif","#ifdef USE_TANGENT","	attribute vec4 tangent;","#endif","#if defined( USE_COLOR_ALPHA )","	attribute vec4 color;","#elif defined( USE_COLOR )","	attribute vec3 color;","#endif","#if ( defined( USE_MORPHTARGETS ) && ! defined( MORPHTARGETS_TEXTURE ) )","	attribute vec3 morphTarget0;","	attribute vec3 morphTarget1;","	attribute vec3 morphTarget2;","	attribute vec3 morphTarget3;","	#ifdef USE_MORPHNORMALS","		attribute vec3 morphNormal0;","		attribute vec3 morphNormal1;","		attribute vec3 morphNormal2;","		attribute vec3 morphNormal3;","	#else","		attribute vec3 morphTarget4;","		attribute vec3 morphTarget5;","		attribute vec3 morphTarget6;","		attribute vec3 morphTarget7;","	#endif","#endif","#ifdef USE_SKINNING","	attribute vec4 skinIndex;","	attribute vec4 skinWeight;","#endif",`
`].filter(Jr).join(`
`),C=[p,yd(e),"#define SHADER_TYPE "+e.shaderType,"#define SHADER_NAME "+e.shaderName,E,e.useFog&&e.fog?"#define USE_FOG":"",e.useFog&&e.fogExp2?"#define FOG_EXP2":"",e.map?"#define USE_MAP":"",e.matcap?"#define USE_MATCAP":"",e.envMap?"#define USE_ENVMAP":"",e.envMap?"#define "+h:"",e.envMap?"#define "+u:"",e.envMap?"#define "+d:"",f?"#define CUBEUV_TEXEL_WIDTH "+f.texelWidth:"",f?"#define CUBEUV_TEXEL_HEIGHT "+f.texelHeight:"",f?"#define CUBEUV_MAX_MIP "+f.maxMip+".0":"",e.lightMap?"#define USE_LIGHTMAP":"",e.aoMap?"#define USE_AOMAP":"",e.bumpMap?"#define USE_BUMPMAP":"",e.normalMap?"#define USE_NORMALMAP":"",e.normalMapObjectSpace?"#define USE_NORMALMAP_OBJECTSPACE":"",e.normalMapTangentSpace?"#define USE_NORMALMAP_TANGENTSPACE":"",e.emissiveMap?"#define USE_EMISSIVEMAP":"",e.anisotropy?"#define USE_ANISOTROPY":"",e.anisotropyMap?"#define USE_ANISOTROPYMAP":"",e.clearcoat?"#define USE_CLEARCOAT":"",e.clearcoatMap?"#define USE_CLEARCOATMAP":"",e.clearcoatRoughnessMap?"#define USE_CLEARCOAT_ROUGHNESSMAP":"",e.clearcoatNormalMap?"#define USE_CLEARCOAT_NORMALMAP":"",e.iridescence?"#define USE_IRIDESCENCE":"",e.iridescenceMap?"#define USE_IRIDESCENCEMAP":"",e.iridescenceThicknessMap?"#define USE_IRIDESCENCE_THICKNESSMAP":"",e.specularMap?"#define USE_SPECULARMAP":"",e.specularColorMap?"#define USE_SPECULAR_COLORMAP":"",e.specularIntensityMap?"#define USE_SPECULAR_INTENSITYMAP":"",e.roughnessMap?"#define USE_ROUGHNESSMAP":"",e.metalnessMap?"#define USE_METALNESSMAP":"",e.alphaMap?"#define USE_ALPHAMAP":"",e.alphaTest?"#define USE_ALPHATEST":"",e.alphaHash?"#define USE_ALPHAHASH":"",e.sheen?"#define USE_SHEEN":"",e.sheenColorMap?"#define USE_SHEEN_COLORMAP":"",e.sheenRoughnessMap?"#define USE_SHEEN_ROUGHNESSMAP":"",e.transmission?"#define USE_TRANSMISSION":"",e.transmissionMap?"#define USE_TRANSMISSIONMAP":"",e.thicknessMap?"#define USE_THICKNESSMAP":"",e.vertexTangents&&e.flatShading===!1?"#define USE_TANGENT":"",e.vertexColors||e.instancingColor?"#define USE_COLOR":"",e.vertexAlphas?"#define USE_COLOR_ALPHA":"",e.vertexUv1s?"#define USE_UV1":"",e.vertexUv2s?"#define USE_UV2":"",e.vertexUv3s?"#define USE_UV3":"",e.pointsUvs?"#define USE_POINTS_UV":"",e.gradientMap?"#define USE_GRADIENTMAP":"",e.flatShading?"#define FLAT_SHADED":"",e.doubleSided?"#define DOUBLE_SIDED":"",e.flipSided?"#define FLIP_SIDED":"",e.shadowMapEnabled?"#define USE_SHADOWMAP":"",e.shadowMapEnabled?"#define "+c:"",e.premultipliedAlpha?"#define PREMULTIPLIED_ALPHA":"",e.numLightProbes>0?"#define USE_LIGHT_PROBES":"",e.useLegacyLights?"#define LEGACY_LIGHTS":"",e.decodeVideoTexture?"#define DECODE_VIDEO_TEXTURE":"",e.logarithmicDepthBuffer?"#define USE_LOGDEPTHBUF":"",e.logarithmicDepthBuffer&&e.rendererExtensionFragDepth?"#define USE_LOGDEPTHBUF_EXT":"","uniform mat4 viewMatrix;","uniform vec3 cameraPosition;","uniform bool isOrthographic;",e.toneMapping!==Li?"#define TONE_MAPPING":"",e.toneMapping!==Li?re.tonemapping_pars_fragment:"",e.toneMapping!==Li?HE("toneMapping",e.toneMapping):"",e.dithering?"#define DITHERING":"",e.opaque?"#define OPAQUE":"",re.colorspace_pars_fragment,kE("linearToOutputTexel",e.outputColorSpace),e.useDepthPacking?"#define DEPTH_PACKING "+e.depthPacking:"",`
`].filter(Jr).join(`
`)),o=pl(o),o=gd(o,e),o=_d(o,e),a=pl(a),a=gd(a,e),a=_d(a,e),o=vd(o),a=vd(a),e.isWebGL2&&e.isRawShaderMaterial!==!0&&(w=`#version 300 es
`,m=[y,"precision mediump sampler2DArray;","#define attribute in","#define varying out","#define texture2D texture"].join(`
`)+`
`+m,C=["precision mediump sampler2DArray;","#define varying in",e.glslVersion===Fu?"":"layout(location = 0) out highp vec4 pc_fragColor;",e.glslVersion===Fu?"":"#define gl_FragColor pc_fragColor","#define gl_FragDepthEXT gl_FragDepth","#define texture2D texture","#define textureCube texture","#define texture2DProj textureProj","#define texture2DLodEXT textureLod","#define texture2DProjLodEXT textureProjLod","#define textureCubeLodEXT textureLod","#define texture2DGradEXT textureGrad","#define texture2DProjGradEXT textureProjGrad","#define textureCubeGradEXT textureGrad"].join(`
`)+`
`+C);const b=w+m+o,N=w+C+a,U=pd(r,r.VERTEX_SHADER,b),T=pd(r,r.FRAGMENT_SHADER,N);r.attachShader(_,U),r.attachShader(_,T),e.index0AttributeName!==void 0?r.bindAttribLocation(_,0,e.index0AttributeName):e.morphTargets===!0&&r.bindAttribLocation(_,0,"position"),r.linkProgram(_);function M(L){if(i.debug.checkShaderErrors){const R=r.getProgramInfoLog(_).trim(),V=r.getShaderInfoLog(U).trim(),G=r.getShaderInfoLog(T).trim();let K=!0,Z=!0;if(r.getProgramParameter(_,r.LINK_STATUS)===!1)if(K=!1,typeof i.debug.onShaderError=="function")i.debug.onShaderError(r,_,U,T);else{const J=md(r,U,"vertex"),et=md(r,T,"fragment");console.error("THREE.WebGLProgram: Shader Error "+r.getError()+" - VALIDATE_STATUS "+r.getProgramParameter(_,r.VALIDATE_STATUS)+`

Program Info Log: `+R+`
`+J+`
`+et)}else R!==""?console.warn("THREE.WebGLProgram: Program Info Log:",R):(V===""||G==="")&&(Z=!1);Z&&(L.diagnostics={runnable:K,programLog:R,vertexShader:{log:V,prefix:m},fragmentShader:{log:G,prefix:C}})}r.deleteShader(U),r.deleteShader(T),v=new pa(r,_),S=qE(r,_)}let v;this.getUniforms=function(){return v===void 0&&M(this),v};let S;this.getAttributes=function(){return S===void 0&&M(this),S};let P=e.rendererExtensionParallelShaderCompile===!1;return this.isReady=function(){return P===!1&&(P=r.getProgramParameter(_,OE)),P},this.destroy=function(){n.releaseStatesOfProgram(this),r.deleteProgram(_),this.program=void 0},this.type=e.shaderType,this.name=e.shaderName,this.id=FE++,this.cacheKey=t,this.usedTimes=1,this.program=_,this.vertexShader=U,this.fragmentShader=T,this}let ix=0;class rx{constructor(){this.shaderCache=new Map,this.materialCache=new Map}update(t){const e=t.vertexShader,n=t.fragmentShader,r=this._getShaderStage(e),s=this._getShaderStage(n),o=this._getShaderCacheForMaterial(t);return o.has(r)===!1&&(o.add(r),r.usedTimes++),o.has(s)===!1&&(o.add(s),s.usedTimes++),this}remove(t){const e=this.materialCache.get(t);for(const n of e)n.usedTimes--,n.usedTimes===0&&this.shaderCache.delete(n.code);return this.materialCache.delete(t),this}getVertexShaderID(t){return this._getShaderStage(t.vertexShader).id}getFragmentShaderID(t){return this._getShaderStage(t.fragmentShader).id}dispose(){this.shaderCache.clear(),this.materialCache.clear()}_getShaderCacheForMaterial(t){const e=this.materialCache;let n=e.get(t);return n===void 0&&(n=new Set,e.set(t,n)),n}_getShaderStage(t){const e=this.shaderCache;let n=e.get(t);return n===void 0&&(n=new sx(t),e.set(t,n)),n}}class sx{constructor(t){this.id=ix++,this.code=t,this.usedTimes=0}}function ox(i,t,e,n,r,s,o){const a=new Hl,c=new rx,h=[],u=r.isWebGL2,d=r.logarithmicDepthBuffer,f=r.vertexTextures;let p=r.precision;const y={MeshDepthMaterial:"depth",MeshDistanceMaterial:"distanceRGBA",MeshNormalMaterial:"normal",MeshBasicMaterial:"basic",MeshLambertMaterial:"lambert",MeshPhongMaterial:"phong",MeshToonMaterial:"toon",MeshStandardMaterial:"physical",MeshPhysicalMaterial:"physical",MeshMatcapMaterial:"matcap",LineBasicMaterial:"basic",LineDashedMaterial:"dashed",PointsMaterial:"points",ShadowMaterial:"shadow",SpriteMaterial:"sprite"};function E(v){return v===0?"uv":`uv${v}`}function _(v,S,P,L,R){const V=L.fog,G=R.geometry,K=v.isMeshStandardMaterial?L.environment:null,Z=(v.isMeshStandardMaterial?e:t).get(v.envMap||K),J=Z&&Z.mapping===Ga?Z.image.height:null,et=y[v.type];v.precision!==null&&(p=r.getMaxPrecision(v.precision),p!==v.precision&&console.warn("THREE.WebGLProgram.getParameters:",v.precision,"not supported, using",p,"instead."));const tt=G.morphAttributes.position||G.morphAttributes.normal||G.morphAttributes.color,pt=tt!==void 0?tt.length:0;let ht=0;G.morphAttributes.position!==void 0&&(ht=1),G.morphAttributes.normal!==void 0&&(ht=2),G.morphAttributes.color!==void 0&&(ht=3);let X,it,mt,Pt;if(et){const Ae=$n[et];X=Ae.vertexShader,it=Ae.fragmentShader}else X=v.vertexShader,it=v.fragmentShader,c.update(v),mt=c.getVertexShaderID(v),Pt=c.getFragmentShaderID(v);const Mt=i.getRenderTarget(),Gt=R.isInstancedMesh===!0,Ft=R.isBatchedMesh===!0,wt=!!v.map,Wt=!!v.matcap,W=!!Z,pe=!!v.aoMap,Rt=!!v.lightMap,kt=!!v.bumpMap,bt=!!v.normalMap,ye=!!v.displacementMap,te=!!v.emissiveMap,O=!!v.metalnessMap,A=!!v.roughnessMap,q=v.anisotropy>0,lt=v.clearcoat>0,rt=v.iridescence>0,ct=v.sheen>0,Ct=v.transmission>0,ut=q&&!!v.anisotropyMap,_t=lt&&!!v.clearcoatMap,Nt=lt&&!!v.clearcoatNormalMap,Qt=lt&&!!v.clearcoatRoughnessMap,st=rt&&!!v.iridescenceMap,he=rt&&!!v.iridescenceThicknessMap,Kt=ct&&!!v.sheenColorMap,qt=ct&&!!v.sheenRoughnessMap,Lt=!!v.specularMap,Et=!!v.specularColorMap,F=!!v.specularIntensityMap,dt=Ct&&!!v.transmissionMap,It=Ct&&!!v.thicknessMap,St=!!v.gradientMap,at=!!v.alphaMap,B=v.alphaTest>0,ft=!!v.alphaHash,yt=!!v.extensions,Ut=!!G.attributes.uv1,Ot=!!G.attributes.uv2,ne=!!G.attributes.uv3;let se=Li;return v.toneMapped&&(Mt===null||Mt.isXRRenderTarget===!0)&&(se=i.toneMapping),{isWebGL2:u,shaderID:et,shaderType:v.type,shaderName:v.name,vertexShader:X,fragmentShader:it,defines:v.defines,customVertexShaderID:mt,customFragmentShaderID:Pt,isRawShaderMaterial:v.isRawShaderMaterial===!0,glslVersion:v.glslVersion,precision:p,batching:Ft,instancing:Gt,instancingColor:Gt&&R.instanceColor!==null,supportsVertexTextures:f,outputColorSpace:Mt===null?i.outputColorSpace:Mt.isXRRenderTarget===!0?Mt.texture.colorSpace:di,map:wt,matcap:Wt,envMap:W,envMapMode:W&&Z.mapping,envMapCubeUVHeight:J,aoMap:pe,lightMap:Rt,bumpMap:kt,normalMap:bt,displacementMap:f&&ye,emissiveMap:te,normalMapObjectSpace:bt&&v.normalMapType===p_,normalMapTangentSpace:bt&&v.normalMapType===np,metalnessMap:O,roughnessMap:A,anisotropy:q,anisotropyMap:ut,clearcoat:lt,clearcoatMap:_t,clearcoatNormalMap:Nt,clearcoatRoughnessMap:Qt,iridescence:rt,iridescenceMap:st,iridescenceThicknessMap:he,sheen:ct,sheenColorMap:Kt,sheenRoughnessMap:qt,specularMap:Lt,specularColorMap:Et,specularIntensityMap:F,transmission:Ct,transmissionMap:dt,thicknessMap:It,gradientMap:St,opaque:v.transparent===!1&&v.blending===ts,alphaMap:at,alphaTest:B,alphaHash:ft,combine:v.combine,mapUv:wt&&E(v.map.channel),aoMapUv:pe&&E(v.aoMap.channel),lightMapUv:Rt&&E(v.lightMap.channel),bumpMapUv:kt&&E(v.bumpMap.channel),normalMapUv:bt&&E(v.normalMap.channel),displacementMapUv:ye&&E(v.displacementMap.channel),emissiveMapUv:te&&E(v.emissiveMap.channel),metalnessMapUv:O&&E(v.metalnessMap.channel),roughnessMapUv:A&&E(v.roughnessMap.channel),anisotropyMapUv:ut&&E(v.anisotropyMap.channel),clearcoatMapUv:_t&&E(v.clearcoatMap.channel),clearcoatNormalMapUv:Nt&&E(v.clearcoatNormalMap.channel),clearcoatRoughnessMapUv:Qt&&E(v.clearcoatRoughnessMap.channel),iridescenceMapUv:st&&E(v.iridescenceMap.channel),iridescenceThicknessMapUv:he&&E(v.iridescenceThicknessMap.channel),sheenColorMapUv:Kt&&E(v.sheenColorMap.channel),sheenRoughnessMapUv:qt&&E(v.sheenRoughnessMap.channel),specularMapUv:Lt&&E(v.specularMap.channel),specularColorMapUv:Et&&E(v.specularColorMap.channel),specularIntensityMapUv:F&&E(v.specularIntensityMap.channel),transmissionMapUv:dt&&E(v.transmissionMap.channel),thicknessMapUv:It&&E(v.thicknessMap.channel),alphaMapUv:at&&E(v.alphaMap.channel),vertexTangents:!!G.attributes.tangent&&(bt||q),vertexColors:v.vertexColors,vertexAlphas:v.vertexColors===!0&&!!G.attributes.color&&G.attributes.color.itemSize===4,vertexUv1s:Ut,vertexUv2s:Ot,vertexUv3s:ne,pointsUvs:R.isPoints===!0&&!!G.attributes.uv&&(wt||at),fog:!!V,useFog:v.fog===!0,fogExp2:V&&V.isFogExp2,flatShading:v.flatShading===!0,sizeAttenuation:v.sizeAttenuation===!0,logarithmicDepthBuffer:d,skinning:R.isSkinnedMesh===!0,morphTargets:G.morphAttributes.position!==void 0,morphNormals:G.morphAttributes.normal!==void 0,morphColors:G.morphAttributes.color!==void 0,morphTargetsCount:pt,morphTextureStride:ht,numDirLights:S.directional.length,numPointLights:S.point.length,numSpotLights:S.spot.length,numSpotLightMaps:S.spotLightMap.length,numRectAreaLights:S.rectArea.length,numHemiLights:S.hemi.length,numDirLightShadows:S.directionalShadowMap.length,numPointLightShadows:S.pointShadowMap.length,numSpotLightShadows:S.spotShadowMap.length,numSpotLightShadowsWithMaps:S.numSpotLightShadowsWithMaps,numLightProbes:S.numLightProbes,numClippingPlanes:o.numPlanes,numClipIntersection:o.numIntersection,dithering:v.dithering,shadowMapEnabled:i.shadowMap.enabled&&P.length>0,shadowMapType:i.shadowMap.type,toneMapping:se,useLegacyLights:i._useLegacyLights,decodeVideoTexture:wt&&v.map.isVideoTexture===!0&&ve.getTransfer(v.map.colorSpace)===we,premultipliedAlpha:v.premultipliedAlpha,doubleSided:v.side===_n,flipSided:v.side===vn,useDepthPacking:v.depthPacking>=0,depthPacking:v.depthPacking||0,index0AttributeName:v.index0AttributeName,extensionDerivatives:yt&&v.extensions.derivatives===!0,extensionFragDepth:yt&&v.extensions.fragDepth===!0,extensionDrawBuffers:yt&&v.extensions.drawBuffers===!0,extensionShaderTextureLOD:yt&&v.extensions.shaderTextureLOD===!0,extensionClipCullDistance:yt&&v.extensions.clipCullDistance&&n.has("WEBGL_clip_cull_distance"),rendererExtensionFragDepth:u||n.has("EXT_frag_depth"),rendererExtensionDrawBuffers:u||n.has("WEBGL_draw_buffers"),rendererExtensionShaderTextureLod:u||n.has("EXT_shader_texture_lod"),rendererExtensionParallelShaderCompile:n.has("KHR_parallel_shader_compile"),customProgramCacheKey:v.customProgramCacheKey()}}function m(v){const S=[];if(v.shaderID?S.push(v.shaderID):(S.push(v.customVertexShaderID),S.push(v.customFragmentShaderID)),v.defines!==void 0)for(const P in v.defines)S.push(P),S.push(v.defines[P]);return v.isRawShaderMaterial===!1&&(C(S,v),w(S,v),S.push(i.outputColorSpace)),S.push(v.customProgramCacheKey),S.join()}function C(v,S){v.push(S.precision),v.push(S.outputColorSpace),v.push(S.envMapMode),v.push(S.envMapCubeUVHeight),v.push(S.mapUv),v.push(S.alphaMapUv),v.push(S.lightMapUv),v.push(S.aoMapUv),v.push(S.bumpMapUv),v.push(S.normalMapUv),v.push(S.displacementMapUv),v.push(S.emissiveMapUv),v.push(S.metalnessMapUv),v.push(S.roughnessMapUv),v.push(S.anisotropyMapUv),v.push(S.clearcoatMapUv),v.push(S.clearcoatNormalMapUv),v.push(S.clearcoatRoughnessMapUv),v.push(S.iridescenceMapUv),v.push(S.iridescenceThicknessMapUv),v.push(S.sheenColorMapUv),v.push(S.sheenRoughnessMapUv),v.push(S.specularMapUv),v.push(S.specularColorMapUv),v.push(S.specularIntensityMapUv),v.push(S.transmissionMapUv),v.push(S.thicknessMapUv),v.push(S.combine),v.push(S.fogExp2),v.push(S.sizeAttenuation),v.push(S.morphTargetsCount),v.push(S.morphAttributeCount),v.push(S.numDirLights),v.push(S.numPointLights),v.push(S.numSpotLights),v.push(S.numSpotLightMaps),v.push(S.numHemiLights),v.push(S.numRectAreaLights),v.push(S.numDirLightShadows),v.push(S.numPointLightShadows),v.push(S.numSpotLightShadows),v.push(S.numSpotLightShadowsWithMaps),v.push(S.numLightProbes),v.push(S.shadowMapType),v.push(S.toneMapping),v.push(S.numClippingPlanes),v.push(S.numClipIntersection),v.push(S.depthPacking)}function w(v,S){a.disableAll(),S.isWebGL2&&a.enable(0),S.supportsVertexTextures&&a.enable(1),S.instancing&&a.enable(2),S.instancingColor&&a.enable(3),S.matcap&&a.enable(4),S.envMap&&a.enable(5),S.normalMapObjectSpace&&a.enable(6),S.normalMapTangentSpace&&a.enable(7),S.clearcoat&&a.enable(8),S.iridescence&&a.enable(9),S.alphaTest&&a.enable(10),S.vertexColors&&a.enable(11),S.vertexAlphas&&a.enable(12),S.vertexUv1s&&a.enable(13),S.vertexUv2s&&a.enable(14),S.vertexUv3s&&a.enable(15),S.vertexTangents&&a.enable(16),S.anisotropy&&a.enable(17),S.alphaHash&&a.enable(18),S.batching&&a.enable(19),v.push(a.mask),a.disableAll(),S.fog&&a.enable(0),S.useFog&&a.enable(1),S.flatShading&&a.enable(2),S.logarithmicDepthBuffer&&a.enable(3),S.skinning&&a.enable(4),S.morphTargets&&a.enable(5),S.morphNormals&&a.enable(6),S.morphColors&&a.enable(7),S.premultipliedAlpha&&a.enable(8),S.shadowMapEnabled&&a.enable(9),S.useLegacyLights&&a.enable(10),S.doubleSided&&a.enable(11),S.flipSided&&a.enable(12),S.useDepthPacking&&a.enable(13),S.dithering&&a.enable(14),S.transmission&&a.enable(15),S.sheen&&a.enable(16),S.opaque&&a.enable(17),S.pointsUvs&&a.enable(18),S.decodeVideoTexture&&a.enable(19),v.push(a.mask)}function b(v){const S=y[v.type];let P;if(S){const L=$n[S];P=z_.clone(L.uniforms)}else P=v.uniforms;return P}function N(v,S){let P;for(let L=0,R=h.length;L<R;L++){const V=h[L];if(V.cacheKey===S){P=V,++P.usedTimes;break}}return P===void 0&&(P=new nx(i,S,v,s),h.push(P)),P}function U(v){if(--v.usedTimes===0){const S=h.indexOf(v);h[S]=h[h.length-1],h.pop(),v.destroy()}}function T(v){c.remove(v)}function M(){c.dispose()}return{getParameters:_,getProgramCacheKey:m,getUniforms:b,acquireProgram:N,releaseProgram:U,releaseShaderCache:T,programs:h,dispose:M}}function ax(){let i=new WeakMap;function t(s){let o=i.get(s);return o===void 0&&(o={},i.set(s,o)),o}function e(s){i.delete(s)}function n(s,o,a){i.get(s)[o]=a}function r(){i=new WeakMap}return{get:t,remove:e,update:n,dispose:r}}function cx(i,t){return i.groupOrder!==t.groupOrder?i.groupOrder-t.groupOrder:i.renderOrder!==t.renderOrder?i.renderOrder-t.renderOrder:i.material.id!==t.material.id?i.material.id-t.material.id:i.z!==t.z?i.z-t.z:i.id-t.id}function Ed(i,t){return i.groupOrder!==t.groupOrder?i.groupOrder-t.groupOrder:i.renderOrder!==t.renderOrder?i.renderOrder-t.renderOrder:i.z!==t.z?t.z-i.z:i.id-t.id}function xd(){const i=[];let t=0;const e=[],n=[],r=[];function s(){t=0,e.length=0,n.length=0,r.length=0}function o(d,f,p,y,E,_){let m=i[t];return m===void 0?(m={id:d.id,object:d,geometry:f,material:p,groupOrder:y,renderOrder:d.renderOrder,z:E,group:_},i[t]=m):(m.id=d.id,m.object=d,m.geometry=f,m.material=p,m.groupOrder=y,m.renderOrder=d.renderOrder,m.z=E,m.group=_),t++,m}function a(d,f,p,y,E,_){const m=o(d,f,p,y,E,_);p.transmission>0?n.push(m):p.transparent===!0?r.push(m):e.push(m)}function c(d,f,p,y,E,_){const m=o(d,f,p,y,E,_);p.transmission>0?n.unshift(m):p.transparent===!0?r.unshift(m):e.unshift(m)}function h(d,f){e.length>1&&e.sort(d||cx),n.length>1&&n.sort(f||Ed),r.length>1&&r.sort(f||Ed)}function u(){for(let d=t,f=i.length;d<f;d++){const p=i[d];if(p.id===null)break;p.id=null,p.object=null,p.geometry=null,p.material=null,p.group=null}}return{opaque:e,transmissive:n,transparent:r,init:s,push:a,unshift:c,finish:u,sort:h}}function lx(){let i=new WeakMap;function t(n,r){const s=i.get(n);let o;return s===void 0?(o=new xd,i.set(n,[o])):r>=s.length?(o=new xd,s.push(o)):o=s[r],o}function e(){i=new WeakMap}return{get:t,dispose:e}}function hx(){const i={};return{get:function(t){if(i[t.id]!==void 0)return i[t.id];let e;switch(t.type){case"DirectionalLight":e={direction:new k,color:new oe};break;case"SpotLight":e={position:new k,direction:new k,color:new oe,distance:0,coneCos:0,penumbraCos:0,decay:0};break;case"PointLight":e={position:new k,color:new oe,distance:0,decay:0};break;case"HemisphereLight":e={direction:new k,skyColor:new oe,groundColor:new oe};break;case"RectAreaLight":e={color:new oe,position:new k,halfWidth:new k,halfHeight:new k};break}return i[t.id]=e,e}}}function ux(){const i={};return{get:function(t){if(i[t.id]!==void 0)return i[t.id];let e;switch(t.type){case"DirectionalLight":e={shadowBias:0,shadowNormalBias:0,shadowRadius:1,shadowMapSize:new Vt};break;case"SpotLight":e={shadowBias:0,shadowNormalBias:0,shadowRadius:1,shadowMapSize:new Vt};break;case"PointLight":e={shadowBias:0,shadowNormalBias:0,shadowRadius:1,shadowMapSize:new Vt,shadowCameraNear:1,shadowCameraFar:1e3};break}return i[t.id]=e,e}}}let dx=0;function fx(i,t){return(t.castShadow?2:0)-(i.castShadow?2:0)+(t.map?1:0)-(i.map?1:0)}function px(i,t){const e=new hx,n=ux(),r={version:0,hash:{directionalLength:-1,pointLength:-1,spotLength:-1,rectAreaLength:-1,hemiLength:-1,numDirectionalShadows:-1,numPointShadows:-1,numSpotShadows:-1,numSpotMaps:-1,numLightProbes:-1},ambient:[0,0,0],probe:[],directional:[],directionalShadow:[],directionalShadowMap:[],directionalShadowMatrix:[],spot:[],spotLightMap:[],spotShadow:[],spotShadowMap:[],spotLightMatrix:[],rectArea:[],rectAreaLTC1:null,rectAreaLTC2:null,point:[],pointShadow:[],pointShadowMap:[],pointShadowMatrix:[],hemi:[],numSpotLightShadowsWithMaps:0,numLightProbes:0};for(let u=0;u<9;u++)r.probe.push(new k);const s=new k,o=new Be,a=new Be;function c(u,d){let f=0,p=0,y=0;for(let L=0;L<9;L++)r.probe[L].set(0,0,0);let E=0,_=0,m=0,C=0,w=0,b=0,N=0,U=0,T=0,M=0,v=0;u.sort(fx);const S=d===!0?Math.PI:1;for(let L=0,R=u.length;L<R;L++){const V=u[L],G=V.color,K=V.intensity,Z=V.distance,J=V.shadow&&V.shadow.map?V.shadow.map.texture:null;if(V.isAmbientLight)f+=G.r*K*S,p+=G.g*K*S,y+=G.b*K*S;else if(V.isLightProbe){for(let et=0;et<9;et++)r.probe[et].addScaledVector(V.sh.coefficients[et],K);v++}else if(V.isDirectionalLight){const et=e.get(V);if(et.color.copy(V.color).multiplyScalar(V.intensity*S),V.castShadow){const tt=V.shadow,pt=n.get(V);pt.shadowBias=tt.bias,pt.shadowNormalBias=tt.normalBias,pt.shadowRadius=tt.radius,pt.shadowMapSize=tt.mapSize,r.directionalShadow[E]=pt,r.directionalShadowMap[E]=J,r.directionalShadowMatrix[E]=V.shadow.matrix,b++}r.directional[E]=et,E++}else if(V.isSpotLight){const et=e.get(V);et.position.setFromMatrixPosition(V.matrixWorld),et.color.copy(G).multiplyScalar(K*S),et.distance=Z,et.coneCos=Math.cos(V.angle),et.penumbraCos=Math.cos(V.angle*(1-V.penumbra)),et.decay=V.decay,r.spot[m]=et;const tt=V.shadow;if(V.map&&(r.spotLightMap[T]=V.map,T++,tt.updateMatrices(V),V.castShadow&&M++),r.spotLightMatrix[m]=tt.matrix,V.castShadow){const pt=n.get(V);pt.shadowBias=tt.bias,pt.shadowNormalBias=tt.normalBias,pt.shadowRadius=tt.radius,pt.shadowMapSize=tt.mapSize,r.spotShadow[m]=pt,r.spotShadowMap[m]=J,U++}m++}else if(V.isRectAreaLight){const et=e.get(V);et.color.copy(G).multiplyScalar(K),et.halfWidth.set(V.width*.5,0,0),et.halfHeight.set(0,V.height*.5,0),r.rectArea[C]=et,C++}else if(V.isPointLight){const et=e.get(V);if(et.color.copy(V.color).multiplyScalar(V.intensity*S),et.distance=V.distance,et.decay=V.decay,V.castShadow){const tt=V.shadow,pt=n.get(V);pt.shadowBias=tt.bias,pt.shadowNormalBias=tt.normalBias,pt.shadowRadius=tt.radius,pt.shadowMapSize=tt.mapSize,pt.shadowCameraNear=tt.camera.near,pt.shadowCameraFar=tt.camera.far,r.pointShadow[_]=pt,r.pointShadowMap[_]=J,r.pointShadowMatrix[_]=V.shadow.matrix,N++}r.point[_]=et,_++}else if(V.isHemisphereLight){const et=e.get(V);et.skyColor.copy(V.color).multiplyScalar(K*S),et.groundColor.copy(V.groundColor).multiplyScalar(K*S),r.hemi[w]=et,w++}}C>0&&(t.isWebGL2?i.has("OES_texture_float_linear")===!0?(r.rectAreaLTC1=vt.LTC_FLOAT_1,r.rectAreaLTC2=vt.LTC_FLOAT_2):(r.rectAreaLTC1=vt.LTC_HALF_1,r.rectAreaLTC2=vt.LTC_HALF_2):i.has("OES_texture_float_linear")===!0?(r.rectAreaLTC1=vt.LTC_FLOAT_1,r.rectAreaLTC2=vt.LTC_FLOAT_2):i.has("OES_texture_half_float_linear")===!0?(r.rectAreaLTC1=vt.LTC_HALF_1,r.rectAreaLTC2=vt.LTC_HALF_2):console.error("THREE.WebGLRenderer: Unable to use RectAreaLight. Missing WebGL extensions.")),r.ambient[0]=f,r.ambient[1]=p,r.ambient[2]=y;const P=r.hash;(P.directionalLength!==E||P.pointLength!==_||P.spotLength!==m||P.rectAreaLength!==C||P.hemiLength!==w||P.numDirectionalShadows!==b||P.numPointShadows!==N||P.numSpotShadows!==U||P.numSpotMaps!==T||P.numLightProbes!==v)&&(r.directional.length=E,r.spot.length=m,r.rectArea.length=C,r.point.length=_,r.hemi.length=w,r.directionalShadow.length=b,r.directionalShadowMap.length=b,r.pointShadow.length=N,r.pointShadowMap.length=N,r.spotShadow.length=U,r.spotShadowMap.length=U,r.directionalShadowMatrix.length=b,r.pointShadowMatrix.length=N,r.spotLightMatrix.length=U+T-M,r.spotLightMap.length=T,r.numSpotLightShadowsWithMaps=M,r.numLightProbes=v,P.directionalLength=E,P.pointLength=_,P.spotLength=m,P.rectAreaLength=C,P.hemiLength=w,P.numDirectionalShadows=b,P.numPointShadows=N,P.numSpotShadows=U,P.numSpotMaps=T,P.numLightProbes=v,r.version=dx++)}function h(u,d){let f=0,p=0,y=0,E=0,_=0;const m=d.matrixWorldInverse;for(let C=0,w=u.length;C<w;C++){const b=u[C];if(b.isDirectionalLight){const N=r.directional[f];N.direction.setFromMatrixPosition(b.matrixWorld),s.setFromMatrixPosition(b.target.matrixWorld),N.direction.sub(s),N.direction.transformDirection(m),f++}else if(b.isSpotLight){const N=r.spot[y];N.position.setFromMatrixPosition(b.matrixWorld),N.position.applyMatrix4(m),N.direction.setFromMatrixPosition(b.matrixWorld),s.setFromMatrixPosition(b.target.matrixWorld),N.direction.sub(s),N.direction.transformDirection(m),y++}else if(b.isRectAreaLight){const N=r.rectArea[E];N.position.setFromMatrixPosition(b.matrixWorld),N.position.applyMatrix4(m),a.identity(),o.copy(b.matrixWorld),o.premultiply(m),a.extractRotation(o),N.halfWidth.set(b.width*.5,0,0),N.halfHeight.set(0,b.height*.5,0),N.halfWidth.applyMatrix4(a),N.halfHeight.applyMatrix4(a),E++}else if(b.isPointLight){const N=r.point[p];N.position.setFromMatrixPosition(b.matrixWorld),N.position.applyMatrix4(m),p++}else if(b.isHemisphereLight){const N=r.hemi[_];N.direction.setFromMatrixPosition(b.matrixWorld),N.direction.transformDirection(m),_++}}}return{setup:c,setupView:h,state:r}}function Sd(i,t){const e=new px(i,t),n=[],r=[];function s(){n.length=0,r.length=0}function o(d){n.push(d)}function a(d){r.push(d)}function c(d){e.setup(n,d)}function h(d){e.setupView(n,d)}return{init:s,state:{lightsArray:n,shadowsArray:r,lights:e},setupLights:c,setupLightsView:h,pushLight:o,pushShadow:a}}function mx(i,t){let e=new WeakMap;function n(s,o=0){const a=e.get(s);let c;return a===void 0?(c=new Sd(i,t),e.set(s,[c])):o>=a.length?(c=new Sd(i,t),a.push(c)):c=a[o],c}function r(){e=new WeakMap}return{get:n,dispose:r}}class gx extends gs{constructor(t){super(),this.isMeshDepthMaterial=!0,this.type="MeshDepthMaterial",this.depthPacking=d_,this.map=null,this.alphaMap=null,this.displacementMap=null,this.displacementScale=1,this.displacementBias=0,this.wireframe=!1,this.wireframeLinewidth=1,this.setValues(t)}copy(t){return super.copy(t),this.depthPacking=t.depthPacking,this.map=t.map,this.alphaMap=t.alphaMap,this.displacementMap=t.displacementMap,this.displacementScale=t.displacementScale,this.displacementBias=t.displacementBias,this.wireframe=t.wireframe,this.wireframeLinewidth=t.wireframeLinewidth,this}}class _x extends gs{constructor(t){super(),this.isMeshDistanceMaterial=!0,this.type="MeshDistanceMaterial",this.map=null,this.alphaMap=null,this.displacementMap=null,this.displacementScale=1,this.displacementBias=0,this.setValues(t)}copy(t){return super.copy(t),this.map=t.map,this.alphaMap=t.alphaMap,this.displacementMap=t.displacementMap,this.displacementScale=t.displacementScale,this.displacementBias=t.displacementBias,this}}const vx=`void main() {
	gl_Position = vec4( position, 1.0 );
}`,yx=`uniform sampler2D shadow_pass;
uniform vec2 resolution;
uniform float radius;
#include <packing>
void main() {
	const float samples = float( VSM_SAMPLES );
	float mean = 0.0;
	float squared_mean = 0.0;
	float uvStride = samples <= 1.0 ? 0.0 : 2.0 / ( samples - 1.0 );
	float uvStart = samples <= 1.0 ? 0.0 : - 1.0;
	for ( float i = 0.0; i < samples; i ++ ) {
		float uvOffset = uvStart + i * uvStride;
		#ifdef HORIZONTAL_PASS
			vec2 distribution = unpackRGBATo2Half( texture2D( shadow_pass, ( gl_FragCoord.xy + vec2( uvOffset, 0.0 ) * radius ) / resolution ) );
			mean += distribution.x;
			squared_mean += distribution.y * distribution.y + distribution.x * distribution.x;
		#else
			float depth = unpackRGBAToDepth( texture2D( shadow_pass, ( gl_FragCoord.xy + vec2( 0.0, uvOffset ) * radius ) / resolution ) );
			mean += depth;
			squared_mean += depth * depth;
		#endif
	}
	mean = mean / samples;
	squared_mean = squared_mean / samples;
	float std_dev = sqrt( squared_mean - mean * mean );
	gl_FragColor = pack2HalfToRGBA( vec2( mean, std_dev ) );
}`;function Ex(i,t,e){let n=new zl;const r=new Vt,s=new Vt,o=new Xe,a=new gx({depthPacking:f_}),c=new _x,h={},u=e.maxTextureSize,d={[ui]:vn,[vn]:ui,[_n]:_n},f=new fi({defines:{VSM_SAMPLES:8},uniforms:{shadow_pass:{value:null},resolution:{value:new Vt},radius:{value:4}},vertexShader:vx,fragmentShader:yx}),p=f.clone();p.defines.HORIZONTAL_PASS=1;const y=new yn;y.setAttribute("position",new xn(new Float32Array([-1,-1,.5,3,-1,.5,-1,3,.5]),3));const E=new Ie(y,f),_=this;this.enabled=!1,this.autoUpdate=!0,this.needsUpdate=!1,this.type=Wf;let m=this.type;this.render=function(U,T,M){if(_.enabled===!1||_.autoUpdate===!1&&_.needsUpdate===!1||U.length===0)return;const v=i.getRenderTarget(),S=i.getActiveCubeFace(),P=i.getActiveMipmapLevel(),L=i.state;L.setBlending(Di),L.buffers.color.setClear(1,1,1,1),L.buffers.depth.setTest(!0),L.setScissorTest(!1);const R=m!==oi&&this.type===oi,V=m===oi&&this.type!==oi;for(let G=0,K=U.length;G<K;G++){const Z=U[G],J=Z.shadow;if(J===void 0){console.warn("THREE.WebGLShadowMap:",Z,"has no shadow.");continue}if(J.autoUpdate===!1&&J.needsUpdate===!1)continue;r.copy(J.mapSize);const et=J.getFrameExtents();if(r.multiply(et),s.copy(J.mapSize),(r.x>u||r.y>u)&&(r.x>u&&(s.x=Math.floor(u/et.x),r.x=s.x*et.x,J.mapSize.x=s.x),r.y>u&&(s.y=Math.floor(u/et.y),r.y=s.y*et.y,J.mapSize.y=s.y)),J.map===null||R===!0||V===!0){const pt=this.type!==oi?{minFilter:pn,magFilter:pn}:{};J.map!==null&&J.map.dispose(),J.map=new hr(r.x,r.y,pt),J.map.texture.name=Z.name+".shadowMap",J.camera.updateProjectionMatrix()}i.setRenderTarget(J.map),i.clear();const tt=J.getViewportCount();for(let pt=0;pt<tt;pt++){const ht=J.getViewport(pt);o.set(s.x*ht.x,s.y*ht.y,s.x*ht.z,s.y*ht.w),L.viewport(o),J.updateMatrices(Z,pt),n=J.getFrustum(),b(T,M,J.camera,Z,this.type)}J.isPointLightShadow!==!0&&this.type===oi&&C(J,M),J.needsUpdate=!1}m=this.type,_.needsUpdate=!1,i.setRenderTarget(v,S,P)};function C(U,T){const M=t.update(E);f.defines.VSM_SAMPLES!==U.blurSamples&&(f.defines.VSM_SAMPLES=U.blurSamples,p.defines.VSM_SAMPLES=U.blurSamples,f.needsUpdate=!0,p.needsUpdate=!0),U.mapPass===null&&(U.mapPass=new hr(r.x,r.y)),f.uniforms.shadow_pass.value=U.map.texture,f.uniforms.resolution.value=U.mapSize,f.uniforms.radius.value=U.radius,i.setRenderTarget(U.mapPass),i.clear(),i.renderBufferDirect(T,null,M,f,E,null),p.uniforms.shadow_pass.value=U.mapPass.texture,p.uniforms.resolution.value=U.mapSize,p.uniforms.radius.value=U.radius,i.setRenderTarget(U.map),i.clear(),i.renderBufferDirect(T,null,M,p,E,null)}function w(U,T,M,v){let S=null;const P=M.isPointLight===!0?U.customDistanceMaterial:U.customDepthMaterial;if(P!==void 0)S=P;else if(S=M.isPointLight===!0?c:a,i.localClippingEnabled&&T.clipShadows===!0&&Array.isArray(T.clippingPlanes)&&T.clippingPlanes.length!==0||T.displacementMap&&T.displacementScale!==0||T.alphaMap&&T.alphaTest>0||T.map&&T.alphaTest>0){const L=S.uuid,R=T.uuid;let V=h[L];V===void 0&&(V={},h[L]=V);let G=V[R];G===void 0&&(G=S.clone(),V[R]=G,T.addEventListener("dispose",N)),S=G}if(S.visible=T.visible,S.wireframe=T.wireframe,v===oi?S.side=T.shadowSide!==null?T.shadowSide:T.side:S.side=T.shadowSide!==null?T.shadowSide:d[T.side],S.alphaMap=T.alphaMap,S.alphaTest=T.alphaTest,S.map=T.map,S.clipShadows=T.clipShadows,S.clippingPlanes=T.clippingPlanes,S.clipIntersection=T.clipIntersection,S.displacementMap=T.displacementMap,S.displacementScale=T.displacementScale,S.displacementBias=T.displacementBias,S.wireframeLinewidth=T.wireframeLinewidth,S.linewidth=T.linewidth,M.isPointLight===!0&&S.isMeshDistanceMaterial===!0){const L=i.properties.get(S);L.light=M}return S}function b(U,T,M,v,S){if(U.visible===!1)return;if(U.layers.test(T.layers)&&(U.isMesh||U.isLine||U.isPoints)&&(U.castShadow||U.receiveShadow&&S===oi)&&(!U.frustumCulled||n.intersectsObject(U))){U.modelViewMatrix.multiplyMatrices(M.matrixWorldInverse,U.matrixWorld);const R=t.update(U),V=U.material;if(Array.isArray(V)){const G=R.groups;for(let K=0,Z=G.length;K<Z;K++){const J=G[K],et=V[J.materialIndex];if(et&&et.visible){const tt=w(U,et,v,S);U.onBeforeShadow(i,U,T,M,R,tt,J),i.renderBufferDirect(M,null,R,tt,U,J),U.onAfterShadow(i,U,T,M,R,tt,J)}}}else if(V.visible){const G=w(U,V,v,S);U.onBeforeShadow(i,U,T,M,R,G,null),i.renderBufferDirect(M,null,R,G,U,null),U.onAfterShadow(i,U,T,M,R,G,null)}}const L=U.children;for(let R=0,V=L.length;R<V;R++)b(L[R],T,M,v,S)}function N(U){U.target.removeEventListener("dispose",N);for(const M in h){const v=h[M],S=U.target.uuid;S in v&&(v[S].dispose(),delete v[S])}}}function xx(i,t,e){const n=e.isWebGL2;function r(){let B=!1;const ft=new Xe;let yt=null;const Ut=new Xe(0,0,0,0);return{setMask:function(Ot){yt!==Ot&&!B&&(i.colorMask(Ot,Ot,Ot,Ot),yt=Ot)},setLocked:function(Ot){B=Ot},setClear:function(Ot,ne,se,_e,Ae){Ae===!0&&(Ot*=_e,ne*=_e,se*=_e),ft.set(Ot,ne,se,_e),Ut.equals(ft)===!1&&(i.clearColor(Ot,ne,se,_e),Ut.copy(ft))},reset:function(){B=!1,yt=null,Ut.set(-1,0,0,0)}}}function s(){let B=!1,ft=null,yt=null,Ut=null;return{setTest:function(Ot){Ot?Ft(i.DEPTH_TEST):wt(i.DEPTH_TEST)},setMask:function(Ot){ft!==Ot&&!B&&(i.depthMask(Ot),ft=Ot)},setFunc:function(Ot){if(yt!==Ot){switch(Ot){case Gg:i.depthFunc(i.NEVER);break;case Wg:i.depthFunc(i.ALWAYS);break;case qg:i.depthFunc(i.LESS);break;case xa:i.depthFunc(i.LEQUAL);break;case Xg:i.depthFunc(i.EQUAL);break;case jg:i.depthFunc(i.GEQUAL);break;case Yg:i.depthFunc(i.GREATER);break;case $g:i.depthFunc(i.NOTEQUAL);break;default:i.depthFunc(i.LEQUAL)}yt=Ot}},setLocked:function(Ot){B=Ot},setClear:function(Ot){Ut!==Ot&&(i.clearDepth(Ot),Ut=Ot)},reset:function(){B=!1,ft=null,yt=null,Ut=null}}}function o(){let B=!1,ft=null,yt=null,Ut=null,Ot=null,ne=null,se=null,_e=null,Ae=null;return{setTest:function(Xt){B||(Xt?Ft(i.STENCIL_TEST):wt(i.STENCIL_TEST))},setMask:function(Xt){ft!==Xt&&!B&&(i.stencilMask(Xt),ft=Xt)},setFunc:function(Xt,Ce,nn){(yt!==Xt||Ut!==Ce||Ot!==nn)&&(i.stencilFunc(Xt,Ce,nn),yt=Xt,Ut=Ce,Ot=nn)},setOp:function(Xt,Ce,nn){(ne!==Xt||se!==Ce||_e!==nn)&&(i.stencilOp(Xt,Ce,nn),ne=Xt,se=Ce,_e=nn)},setLocked:function(Xt){B=Xt},setClear:function(Xt){Ae!==Xt&&(i.clearStencil(Xt),Ae=Xt)},reset:function(){B=!1,ft=null,yt=null,Ut=null,Ot=null,ne=null,se=null,_e=null,Ae=null}}}const a=new r,c=new s,h=new o,u=new WeakMap,d=new WeakMap;let f={},p={},y=new WeakMap,E=[],_=null,m=!1,C=null,w=null,b=null,N=null,U=null,T=null,M=null,v=new oe(0,0,0),S=0,P=!1,L=null,R=null,V=null,G=null,K=null;const Z=i.getParameter(i.MAX_COMBINED_TEXTURE_IMAGE_UNITS);let J=!1,et=0;const tt=i.getParameter(i.VERSION);tt.indexOf("WebGL")!==-1?(et=parseFloat(/^WebGL (\d)/.exec(tt)[1]),J=et>=1):tt.indexOf("OpenGL ES")!==-1&&(et=parseFloat(/^OpenGL ES (\d)/.exec(tt)[1]),J=et>=2);let pt=null,ht={};const X=i.getParameter(i.SCISSOR_BOX),it=i.getParameter(i.VIEWPORT),mt=new Xe().fromArray(X),Pt=new Xe().fromArray(it);function Mt(B,ft,yt,Ut){const Ot=new Uint8Array(4),ne=i.createTexture();i.bindTexture(B,ne),i.texParameteri(B,i.TEXTURE_MIN_FILTER,i.NEAREST),i.texParameteri(B,i.TEXTURE_MAG_FILTER,i.NEAREST);for(let se=0;se<yt;se++)n&&(B===i.TEXTURE_3D||B===i.TEXTURE_2D_ARRAY)?i.texImage3D(ft,0,i.RGBA,1,1,Ut,0,i.RGBA,i.UNSIGNED_BYTE,Ot):i.texImage2D(ft+se,0,i.RGBA,1,1,0,i.RGBA,i.UNSIGNED_BYTE,Ot);return ne}const Gt={};Gt[i.TEXTURE_2D]=Mt(i.TEXTURE_2D,i.TEXTURE_2D,1),Gt[i.TEXTURE_CUBE_MAP]=Mt(i.TEXTURE_CUBE_MAP,i.TEXTURE_CUBE_MAP_POSITIVE_X,6),n&&(Gt[i.TEXTURE_2D_ARRAY]=Mt(i.TEXTURE_2D_ARRAY,i.TEXTURE_2D_ARRAY,1,1),Gt[i.TEXTURE_3D]=Mt(i.TEXTURE_3D,i.TEXTURE_3D,1,1)),a.setClear(0,0,0,1),c.setClear(1),h.setClear(0),Ft(i.DEPTH_TEST),c.setFunc(xa),te(!1),O(ru),Ft(i.CULL_FACE),bt(Di);function Ft(B){f[B]!==!0&&(i.enable(B),f[B]=!0)}function wt(B){f[B]!==!1&&(i.disable(B),f[B]=!1)}function Wt(B,ft){return p[B]!==ft?(i.bindFramebuffer(B,ft),p[B]=ft,n&&(B===i.DRAW_FRAMEBUFFER&&(p[i.FRAMEBUFFER]=ft),B===i.FRAMEBUFFER&&(p[i.DRAW_FRAMEBUFFER]=ft)),!0):!1}function W(B,ft){let yt=E,Ut=!1;if(B)if(yt=y.get(ft),yt===void 0&&(yt=[],y.set(ft,yt)),B.isWebGLMultipleRenderTargets){const Ot=B.texture;if(yt.length!==Ot.length||yt[0]!==i.COLOR_ATTACHMENT0){for(let ne=0,se=Ot.length;ne<se;ne++)yt[ne]=i.COLOR_ATTACHMENT0+ne;yt.length=Ot.length,Ut=!0}}else yt[0]!==i.COLOR_ATTACHMENT0&&(yt[0]=i.COLOR_ATTACHMENT0,Ut=!0);else yt[0]!==i.BACK&&(yt[0]=i.BACK,Ut=!0);Ut&&(e.isWebGL2?i.drawBuffers(yt):t.get("WEBGL_draw_buffers").drawBuffersWEBGL(yt))}function pe(B){return _!==B?(i.useProgram(B),_=B,!0):!1}const Rt={[nr]:i.FUNC_ADD,[Rg]:i.FUNC_SUBTRACT,[Cg]:i.FUNC_REVERSE_SUBTRACT};if(n)Rt[cu]=i.MIN,Rt[lu]=i.MAX;else{const B=t.get("EXT_blend_minmax");B!==null&&(Rt[cu]=B.MIN_EXT,Rt[lu]=B.MAX_EXT)}const kt={[Ig]:i.ZERO,[Pg]:i.ONE,[Dg]:i.SRC_COLOR,[rl]:i.SRC_ALPHA,[Vg]:i.SRC_ALPHA_SATURATE,[Og]:i.DST_COLOR,[Ng]:i.DST_ALPHA,[Lg]:i.ONE_MINUS_SRC_COLOR,[sl]:i.ONE_MINUS_SRC_ALPHA,[Fg]:i.ONE_MINUS_DST_COLOR,[Ug]:i.ONE_MINUS_DST_ALPHA,[Bg]:i.CONSTANT_COLOR,[kg]:i.ONE_MINUS_CONSTANT_COLOR,[Hg]:i.CONSTANT_ALPHA,[zg]:i.ONE_MINUS_CONSTANT_ALPHA};function bt(B,ft,yt,Ut,Ot,ne,se,_e,Ae,Xt){if(B===Di){m===!0&&(wt(i.BLEND),m=!1);return}if(m===!1&&(Ft(i.BLEND),m=!0),B!==Ag){if(B!==C||Xt!==P){if((w!==nr||U!==nr)&&(i.blendEquation(i.FUNC_ADD),w=nr,U=nr),Xt)switch(B){case ts:i.blendFuncSeparate(i.ONE,i.ONE_MINUS_SRC_ALPHA,i.ONE,i.ONE_MINUS_SRC_ALPHA);break;case su:i.blendFunc(i.ONE,i.ONE);break;case ou:i.blendFuncSeparate(i.ZERO,i.ONE_MINUS_SRC_COLOR,i.ZERO,i.ONE);break;case au:i.blendFuncSeparate(i.ZERO,i.SRC_COLOR,i.ZERO,i.SRC_ALPHA);break;default:console.error("THREE.WebGLState: Invalid blending: ",B);break}else switch(B){case ts:i.blendFuncSeparate(i.SRC_ALPHA,i.ONE_MINUS_SRC_ALPHA,i.ONE,i.ONE_MINUS_SRC_ALPHA);break;case su:i.blendFunc(i.SRC_ALPHA,i.ONE);break;case ou:i.blendFuncSeparate(i.ZERO,i.ONE_MINUS_SRC_COLOR,i.ZERO,i.ONE);break;case au:i.blendFunc(i.ZERO,i.SRC_COLOR);break;default:console.error("THREE.WebGLState: Invalid blending: ",B);break}b=null,N=null,T=null,M=null,v.set(0,0,0),S=0,C=B,P=Xt}return}Ot=Ot||ft,ne=ne||yt,se=se||Ut,(ft!==w||Ot!==U)&&(i.blendEquationSeparate(Rt[ft],Rt[Ot]),w=ft,U=Ot),(yt!==b||Ut!==N||ne!==T||se!==M)&&(i.blendFuncSeparate(kt[yt],kt[Ut],kt[ne],kt[se]),b=yt,N=Ut,T=ne,M=se),(_e.equals(v)===!1||Ae!==S)&&(i.blendColor(_e.r,_e.g,_e.b,Ae),v.copy(_e),S=Ae),C=B,P=!1}function ye(B,ft){B.side===_n?wt(i.CULL_FACE):Ft(i.CULL_FACE);let yt=B.side===vn;ft&&(yt=!yt),te(yt),B.blending===ts&&B.transparent===!1?bt(Di):bt(B.blending,B.blendEquation,B.blendSrc,B.blendDst,B.blendEquationAlpha,B.blendSrcAlpha,B.blendDstAlpha,B.blendColor,B.blendAlpha,B.premultipliedAlpha),c.setFunc(B.depthFunc),c.setTest(B.depthTest),c.setMask(B.depthWrite),a.setMask(B.colorWrite);const Ut=B.stencilWrite;h.setTest(Ut),Ut&&(h.setMask(B.stencilWriteMask),h.setFunc(B.stencilFunc,B.stencilRef,B.stencilFuncMask),h.setOp(B.stencilFail,B.stencilZFail,B.stencilZPass)),q(B.polygonOffset,B.polygonOffsetFactor,B.polygonOffsetUnits),B.alphaToCoverage===!0?Ft(i.SAMPLE_ALPHA_TO_COVERAGE):wt(i.SAMPLE_ALPHA_TO_COVERAGE)}function te(B){L!==B&&(B?i.frontFace(i.CW):i.frontFace(i.CCW),L=B)}function O(B){B!==Mg?(Ft(i.CULL_FACE),B!==R&&(B===ru?i.cullFace(i.BACK):B===wg?i.cullFace(i.FRONT):i.cullFace(i.FRONT_AND_BACK))):wt(i.CULL_FACE),R=B}function A(B){B!==V&&(J&&i.lineWidth(B),V=B)}function q(B,ft,yt){B?(Ft(i.POLYGON_OFFSET_FILL),(G!==ft||K!==yt)&&(i.polygonOffset(ft,yt),G=ft,K=yt)):wt(i.POLYGON_OFFSET_FILL)}function lt(B){B?Ft(i.SCISSOR_TEST):wt(i.SCISSOR_TEST)}function rt(B){B===void 0&&(B=i.TEXTURE0+Z-1),pt!==B&&(i.activeTexture(B),pt=B)}function ct(B,ft,yt){yt===void 0&&(pt===null?yt=i.TEXTURE0+Z-1:yt=pt);let Ut=ht[yt];Ut===void 0&&(Ut={type:void 0,texture:void 0},ht[yt]=Ut),(Ut.type!==B||Ut.texture!==ft)&&(pt!==yt&&(i.activeTexture(yt),pt=yt),i.bindTexture(B,ft||Gt[B]),Ut.type=B,Ut.texture=ft)}function Ct(){const B=ht[pt];B!==void 0&&B.type!==void 0&&(i.bindTexture(B.type,null),B.type=void 0,B.texture=void 0)}function ut(){try{i.compressedTexImage2D.apply(i,arguments)}catch(B){console.error("THREE.WebGLState:",B)}}function _t(){try{i.compressedTexImage3D.apply(i,arguments)}catch(B){console.error("THREE.WebGLState:",B)}}function Nt(){try{i.texSubImage2D.apply(i,arguments)}catch(B){console.error("THREE.WebGLState:",B)}}function Qt(){try{i.texSubImage3D.apply(i,arguments)}catch(B){console.error("THREE.WebGLState:",B)}}function st(){try{i.compressedTexSubImage2D.apply(i,arguments)}catch(B){console.error("THREE.WebGLState:",B)}}function he(){try{i.compressedTexSubImage3D.apply(i,arguments)}catch(B){console.error("THREE.WebGLState:",B)}}function Kt(){try{i.texStorage2D.apply(i,arguments)}catch(B){console.error("THREE.WebGLState:",B)}}function qt(){try{i.texStorage3D.apply(i,arguments)}catch(B){console.error("THREE.WebGLState:",B)}}function Lt(){try{i.texImage2D.apply(i,arguments)}catch(B){console.error("THREE.WebGLState:",B)}}function Et(){try{i.texImage3D.apply(i,arguments)}catch(B){console.error("THREE.WebGLState:",B)}}function F(B){mt.equals(B)===!1&&(i.scissor(B.x,B.y,B.z,B.w),mt.copy(B))}function dt(B){Pt.equals(B)===!1&&(i.viewport(B.x,B.y,B.z,B.w),Pt.copy(B))}function It(B,ft){let yt=d.get(ft);yt===void 0&&(yt=new WeakMap,d.set(ft,yt));let Ut=yt.get(B);Ut===void 0&&(Ut=i.getUniformBlockIndex(ft,B.name),yt.set(B,Ut))}function St(B,ft){const Ut=d.get(ft).get(B);u.get(ft)!==Ut&&(i.uniformBlockBinding(ft,Ut,B.__bindingPointIndex),u.set(ft,Ut))}function at(){i.disable(i.BLEND),i.disable(i.CULL_FACE),i.disable(i.DEPTH_TEST),i.disable(i.POLYGON_OFFSET_FILL),i.disable(i.SCISSOR_TEST),i.disable(i.STENCIL_TEST),i.disable(i.SAMPLE_ALPHA_TO_COVERAGE),i.blendEquation(i.FUNC_ADD),i.blendFunc(i.ONE,i.ZERO),i.blendFuncSeparate(i.ONE,i.ZERO,i.ONE,i.ZERO),i.blendColor(0,0,0,0),i.colorMask(!0,!0,!0,!0),i.clearColor(0,0,0,0),i.depthMask(!0),i.depthFunc(i.LESS),i.clearDepth(1),i.stencilMask(4294967295),i.stencilFunc(i.ALWAYS,0,4294967295),i.stencilOp(i.KEEP,i.KEEP,i.KEEP),i.clearStencil(0),i.cullFace(i.BACK),i.frontFace(i.CCW),i.polygonOffset(0,0),i.activeTexture(i.TEXTURE0),i.bindFramebuffer(i.FRAMEBUFFER,null),n===!0&&(i.bindFramebuffer(i.DRAW_FRAMEBUFFER,null),i.bindFramebuffer(i.READ_FRAMEBUFFER,null)),i.useProgram(null),i.lineWidth(1),i.scissor(0,0,i.canvas.width,i.canvas.height),i.viewport(0,0,i.canvas.width,i.canvas.height),f={},pt=null,ht={},p={},y=new WeakMap,E=[],_=null,m=!1,C=null,w=null,b=null,N=null,U=null,T=null,M=null,v=new oe(0,0,0),S=0,P=!1,L=null,R=null,V=null,G=null,K=null,mt.set(0,0,i.canvas.width,i.canvas.height),Pt.set(0,0,i.canvas.width,i.canvas.height),a.reset(),c.reset(),h.reset()}return{buffers:{color:a,depth:c,stencil:h},enable:Ft,disable:wt,bindFramebuffer:Wt,drawBuffers:W,useProgram:pe,setBlending:bt,setMaterial:ye,setFlipSided:te,setCullFace:O,setLineWidth:A,setPolygonOffset:q,setScissorTest:lt,activeTexture:rt,bindTexture:ct,unbindTexture:Ct,compressedTexImage2D:ut,compressedTexImage3D:_t,texImage2D:Lt,texImage3D:Et,updateUBOMapping:It,uniformBlockBinding:St,texStorage2D:Kt,texStorage3D:qt,texSubImage2D:Nt,texSubImage3D:Qt,compressedTexSubImage2D:st,compressedTexSubImage3D:he,scissor:F,viewport:dt,reset:at}}function Sx(i,t,e,n,r,s,o){const a=r.isWebGL2,c=t.has("WEBGL_multisampled_render_to_texture")?t.get("WEBGL_multisampled_render_to_texture"):null,h=typeof navigator>"u"?!1:/OculusBrowser/g.test(navigator.userAgent),u=new WeakMap;let d;const f=new WeakMap;let p=!1;try{p=typeof OffscreenCanvas<"u"&&new OffscreenCanvas(1,1).getContext("2d")!==null}catch{}function y(O,A){return p?new OffscreenCanvas(O,A):ba("canvas")}function E(O,A,q,lt){let rt=1;if((O.width>lt||O.height>lt)&&(rt=lt/Math.max(O.width,O.height)),rt<1||A===!0)if(typeof HTMLImageElement<"u"&&O instanceof HTMLImageElement||typeof HTMLCanvasElement<"u"&&O instanceof HTMLCanvasElement||typeof ImageBitmap<"u"&&O instanceof ImageBitmap){const ct=A?fl:Math.floor,Ct=ct(rt*O.width),ut=ct(rt*O.height);d===void 0&&(d=y(Ct,ut));const _t=q?y(Ct,ut):d;return _t.width=Ct,_t.height=ut,_t.getContext("2d").drawImage(O,0,0,Ct,ut),console.warn("THREE.WebGLRenderer: Texture has been resized from ("+O.width+"x"+O.height+") to ("+Ct+"x"+ut+")."),_t}else return"data"in O&&console.warn("THREE.WebGLRenderer: Image in DataTexture is too big ("+O.width+"x"+O.height+")."),O;return O}function _(O){return Vu(O.width)&&Vu(O.height)}function m(O){return a?!1:O.wrapS!==Gn||O.wrapT!==Gn||O.minFilter!==pn&&O.minFilter!==Pn}function C(O,A){return O.generateMipmaps&&A&&O.minFilter!==pn&&O.minFilter!==Pn}function w(O){i.generateMipmap(O)}function b(O,A,q,lt,rt=!1){if(a===!1)return A;if(O!==null){if(i[O]!==void 0)return i[O];console.warn("THREE.WebGLRenderer: Attempt to use non-existing WebGL internal format '"+O+"'")}let ct=A;if(A===i.RED&&(q===i.FLOAT&&(ct=i.R32F),q===i.HALF_FLOAT&&(ct=i.R16F),q===i.UNSIGNED_BYTE&&(ct=i.R8)),A===i.RED_INTEGER&&(q===i.UNSIGNED_BYTE&&(ct=i.R8UI),q===i.UNSIGNED_SHORT&&(ct=i.R16UI),q===i.UNSIGNED_INT&&(ct=i.R32UI),q===i.BYTE&&(ct=i.R8I),q===i.SHORT&&(ct=i.R16I),q===i.INT&&(ct=i.R32I)),A===i.RG&&(q===i.FLOAT&&(ct=i.RG32F),q===i.HALF_FLOAT&&(ct=i.RG16F),q===i.UNSIGNED_BYTE&&(ct=i.RG8)),A===i.RGBA){const Ct=rt?Sa:ve.getTransfer(lt);q===i.FLOAT&&(ct=i.RGBA32F),q===i.HALF_FLOAT&&(ct=i.RGBA16F),q===i.UNSIGNED_BYTE&&(ct=Ct===we?i.SRGB8_ALPHA8:i.RGBA8),q===i.UNSIGNED_SHORT_4_4_4_4&&(ct=i.RGBA4),q===i.UNSIGNED_SHORT_5_5_5_1&&(ct=i.RGB5_A1)}return(ct===i.R16F||ct===i.R32F||ct===i.RG16F||ct===i.RG32F||ct===i.RGBA16F||ct===i.RGBA32F)&&t.get("EXT_color_buffer_float"),ct}function N(O,A,q){return C(O,q)===!0||O.isFramebufferTexture&&O.minFilter!==pn&&O.minFilter!==Pn?Math.log2(Math.max(A.width,A.height))+1:O.mipmaps!==void 0&&O.mipmaps.length>0?O.mipmaps.length:O.isCompressedTexture&&Array.isArray(O.image)?A.mipmaps.length:1}function U(O){return O===pn||O===hu||O===dc?i.NEAREST:i.LINEAR}function T(O){const A=O.target;A.removeEventListener("dispose",T),v(A),A.isVideoTexture&&u.delete(A)}function M(O){const A=O.target;A.removeEventListener("dispose",M),P(A)}function v(O){const A=n.get(O);if(A.__webglInit===void 0)return;const q=O.source,lt=f.get(q);if(lt){const rt=lt[A.__cacheKey];rt.usedTimes--,rt.usedTimes===0&&S(O),Object.keys(lt).length===0&&f.delete(q)}n.remove(O)}function S(O){const A=n.get(O);i.deleteTexture(A.__webglTexture);const q=O.source,lt=f.get(q);delete lt[A.__cacheKey],o.memory.textures--}function P(O){const A=O.texture,q=n.get(O),lt=n.get(A);if(lt.__webglTexture!==void 0&&(i.deleteTexture(lt.__webglTexture),o.memory.textures--),O.depthTexture&&O.depthTexture.dispose(),O.isWebGLCubeRenderTarget)for(let rt=0;rt<6;rt++){if(Array.isArray(q.__webglFramebuffer[rt]))for(let ct=0;ct<q.__webglFramebuffer[rt].length;ct++)i.deleteFramebuffer(q.__webglFramebuffer[rt][ct]);else i.deleteFramebuffer(q.__webglFramebuffer[rt]);q.__webglDepthbuffer&&i.deleteRenderbuffer(q.__webglDepthbuffer[rt])}else{if(Array.isArray(q.__webglFramebuffer))for(let rt=0;rt<q.__webglFramebuffer.length;rt++)i.deleteFramebuffer(q.__webglFramebuffer[rt]);else i.deleteFramebuffer(q.__webglFramebuffer);if(q.__webglDepthbuffer&&i.deleteRenderbuffer(q.__webglDepthbuffer),q.__webglMultisampledFramebuffer&&i.deleteFramebuffer(q.__webglMultisampledFramebuffer),q.__webglColorRenderbuffer)for(let rt=0;rt<q.__webglColorRenderbuffer.length;rt++)q.__webglColorRenderbuffer[rt]&&i.deleteRenderbuffer(q.__webglColorRenderbuffer[rt]);q.__webglDepthRenderbuffer&&i.deleteRenderbuffer(q.__webglDepthRenderbuffer)}if(O.isWebGLMultipleRenderTargets)for(let rt=0,ct=A.length;rt<ct;rt++){const Ct=n.get(A[rt]);Ct.__webglTexture&&(i.deleteTexture(Ct.__webglTexture),o.memory.textures--),n.remove(A[rt])}n.remove(A),n.remove(O)}let L=0;function R(){L=0}function V(){const O=L;return O>=r.maxTextures&&console.warn("THREE.WebGLTextures: Trying to use "+O+" texture units while this GPU supports only "+r.maxTextures),L+=1,O}function G(O){const A=[];return A.push(O.wrapS),A.push(O.wrapT),A.push(O.wrapR||0),A.push(O.magFilter),A.push(O.minFilter),A.push(O.anisotropy),A.push(O.internalFormat),A.push(O.format),A.push(O.type),A.push(O.generateMipmaps),A.push(O.premultiplyAlpha),A.push(O.flipY),A.push(O.unpackAlignment),A.push(O.colorSpace),A.join()}function K(O,A){const q=n.get(O);if(O.isVideoTexture&&ye(O),O.isRenderTargetTexture===!1&&O.version>0&&q.__version!==O.version){const lt=O.image;if(lt===null)console.warn("THREE.WebGLRenderer: Texture marked for update but no image data found.");else if(lt.complete===!1)console.warn("THREE.WebGLRenderer: Texture marked for update but image is incomplete");else{mt(q,O,A);return}}e.bindTexture(i.TEXTURE_2D,q.__webglTexture,i.TEXTURE0+A)}function Z(O,A){const q=n.get(O);if(O.version>0&&q.__version!==O.version){mt(q,O,A);return}e.bindTexture(i.TEXTURE_2D_ARRAY,q.__webglTexture,i.TEXTURE0+A)}function J(O,A){const q=n.get(O);if(O.version>0&&q.__version!==O.version){mt(q,O,A);return}e.bindTexture(i.TEXTURE_3D,q.__webglTexture,i.TEXTURE0+A)}function et(O,A){const q=n.get(O);if(O.version>0&&q.__version!==O.version){Pt(q,O,A);return}e.bindTexture(i.TEXTURE_CUBE_MAP,q.__webglTexture,i.TEXTURE0+A)}const tt={[cl]:i.REPEAT,[Gn]:i.CLAMP_TO_EDGE,[ll]:i.MIRRORED_REPEAT},pt={[pn]:i.NEAREST,[hu]:i.NEAREST_MIPMAP_NEAREST,[dc]:i.NEAREST_MIPMAP_LINEAR,[Pn]:i.LINEAR,[i_]:i.LINEAR_MIPMAP_NEAREST,[to]:i.LINEAR_MIPMAP_LINEAR},ht={[m_]:i.NEVER,[x_]:i.ALWAYS,[g_]:i.LESS,[ip]:i.LEQUAL,[__]:i.EQUAL,[E_]:i.GEQUAL,[v_]:i.GREATER,[y_]:i.NOTEQUAL};function X(O,A,q){if(q?(i.texParameteri(O,i.TEXTURE_WRAP_S,tt[A.wrapS]),i.texParameteri(O,i.TEXTURE_WRAP_T,tt[A.wrapT]),(O===i.TEXTURE_3D||O===i.TEXTURE_2D_ARRAY)&&i.texParameteri(O,i.TEXTURE_WRAP_R,tt[A.wrapR]),i.texParameteri(O,i.TEXTURE_MAG_FILTER,pt[A.magFilter]),i.texParameteri(O,i.TEXTURE_MIN_FILTER,pt[A.minFilter])):(i.texParameteri(O,i.TEXTURE_WRAP_S,i.CLAMP_TO_EDGE),i.texParameteri(O,i.TEXTURE_WRAP_T,i.CLAMP_TO_EDGE),(O===i.TEXTURE_3D||O===i.TEXTURE_2D_ARRAY)&&i.texParameteri(O,i.TEXTURE_WRAP_R,i.CLAMP_TO_EDGE),(A.wrapS!==Gn||A.wrapT!==Gn)&&console.warn("THREE.WebGLRenderer: Texture is not power of two. Texture.wrapS and Texture.wrapT should be set to THREE.ClampToEdgeWrapping."),i.texParameteri(O,i.TEXTURE_MAG_FILTER,U(A.magFilter)),i.texParameteri(O,i.TEXTURE_MIN_FILTER,U(A.minFilter)),A.minFilter!==pn&&A.minFilter!==Pn&&console.warn("THREE.WebGLRenderer: Texture is not power of two. Texture.minFilter should be set to THREE.NearestFilter or THREE.LinearFilter.")),A.compareFunction&&(i.texParameteri(O,i.TEXTURE_COMPARE_MODE,i.COMPARE_REF_TO_TEXTURE),i.texParameteri(O,i.TEXTURE_COMPARE_FUNC,ht[A.compareFunction])),t.has("EXT_texture_filter_anisotropic")===!0){const lt=t.get("EXT_texture_filter_anisotropic");if(A.magFilter===pn||A.minFilter!==dc&&A.minFilter!==to||A.type===Ci&&t.has("OES_texture_float_linear")===!1||a===!1&&A.type===eo&&t.has("OES_texture_half_float_linear")===!1)return;(A.anisotropy>1||n.get(A).__currentAnisotropy)&&(i.texParameterf(O,lt.TEXTURE_MAX_ANISOTROPY_EXT,Math.min(A.anisotropy,r.getMaxAnisotropy())),n.get(A).__currentAnisotropy=A.anisotropy)}}function it(O,A){let q=!1;O.__webglInit===void 0&&(O.__webglInit=!0,A.addEventListener("dispose",T));const lt=A.source;let rt=f.get(lt);rt===void 0&&(rt={},f.set(lt,rt));const ct=G(A);if(ct!==O.__cacheKey){rt[ct]===void 0&&(rt[ct]={texture:i.createTexture(),usedTimes:0},o.memory.textures++,q=!0),rt[ct].usedTimes++;const Ct=rt[O.__cacheKey];Ct!==void 0&&(rt[O.__cacheKey].usedTimes--,Ct.usedTimes===0&&S(A)),O.__cacheKey=ct,O.__webglTexture=rt[ct].texture}return q}function mt(O,A,q){let lt=i.TEXTURE_2D;(A.isDataArrayTexture||A.isCompressedArrayTexture)&&(lt=i.TEXTURE_2D_ARRAY),A.isData3DTexture&&(lt=i.TEXTURE_3D);const rt=it(O,A),ct=A.source;e.bindTexture(lt,O.__webglTexture,i.TEXTURE0+q);const Ct=n.get(ct);if(ct.version!==Ct.__version||rt===!0){e.activeTexture(i.TEXTURE0+q);const ut=ve.getPrimaries(ve.workingColorSpace),_t=A.colorSpace===Nn?null:ve.getPrimaries(A.colorSpace),Nt=A.colorSpace===Nn||ut===_t?i.NONE:i.BROWSER_DEFAULT_WEBGL;i.pixelStorei(i.UNPACK_FLIP_Y_WEBGL,A.flipY),i.pixelStorei(i.UNPACK_PREMULTIPLY_ALPHA_WEBGL,A.premultiplyAlpha),i.pixelStorei(i.UNPACK_ALIGNMENT,A.unpackAlignment),i.pixelStorei(i.UNPACK_COLORSPACE_CONVERSION_WEBGL,Nt);const Qt=m(A)&&_(A.image)===!1;let st=E(A.image,Qt,!1,r.maxTextureSize);st=te(A,st);const he=_(st)||a,Kt=s.convert(A.format,A.colorSpace);let qt=s.convert(A.type),Lt=b(A.internalFormat,Kt,qt,A.colorSpace,A.isVideoTexture);X(lt,A,he);let Et;const F=A.mipmaps,dt=a&&A.isVideoTexture!==!0&&Lt!==tp,It=Ct.__version===void 0||rt===!0,St=N(A,st,he);if(A.isDepthTexture)Lt=i.DEPTH_COMPONENT,a?A.type===Ci?Lt=i.DEPTH_COMPONENT32F:A.type===Ri?Lt=i.DEPTH_COMPONENT24:A.type===or?Lt=i.DEPTH24_STENCIL8:Lt=i.DEPTH_COMPONENT16:A.type===Ci&&console.error("WebGLRenderer: Floating point depth texture requires WebGL2."),A.format===ar&&Lt===i.DEPTH_COMPONENT&&A.type!==Fl&&A.type!==Ri&&(console.warn("THREE.WebGLRenderer: Use UnsignedShortType or UnsignedIntType for DepthFormat DepthTexture."),A.type=Ri,qt=s.convert(A.type)),A.format===os&&Lt===i.DEPTH_COMPONENT&&(Lt=i.DEPTH_STENCIL,A.type!==or&&(console.warn("THREE.WebGLRenderer: Use UnsignedInt248Type for DepthStencilFormat DepthTexture."),A.type=or,qt=s.convert(A.type))),It&&(dt?e.texStorage2D(i.TEXTURE_2D,1,Lt,st.width,st.height):e.texImage2D(i.TEXTURE_2D,0,Lt,st.width,st.height,0,Kt,qt,null));else if(A.isDataTexture)if(F.length>0&&he){dt&&It&&e.texStorage2D(i.TEXTURE_2D,St,Lt,F[0].width,F[0].height);for(let at=0,B=F.length;at<B;at++)Et=F[at],dt?e.texSubImage2D(i.TEXTURE_2D,at,0,0,Et.width,Et.height,Kt,qt,Et.data):e.texImage2D(i.TEXTURE_2D,at,Lt,Et.width,Et.height,0,Kt,qt,Et.data);A.generateMipmaps=!1}else dt?(It&&e.texStorage2D(i.TEXTURE_2D,St,Lt,st.width,st.height),e.texSubImage2D(i.TEXTURE_2D,0,0,0,st.width,st.height,Kt,qt,st.data)):e.texImage2D(i.TEXTURE_2D,0,Lt,st.width,st.height,0,Kt,qt,st.data);else if(A.isCompressedTexture)if(A.isCompressedArrayTexture){dt&&It&&e.texStorage3D(i.TEXTURE_2D_ARRAY,St,Lt,F[0].width,F[0].height,st.depth);for(let at=0,B=F.length;at<B;at++)Et=F[at],A.format!==Wn?Kt!==null?dt?e.compressedTexSubImage3D(i.TEXTURE_2D_ARRAY,at,0,0,0,Et.width,Et.height,st.depth,Kt,Et.data,0,0):e.compressedTexImage3D(i.TEXTURE_2D_ARRAY,at,Lt,Et.width,Et.height,st.depth,0,Et.data,0,0):console.warn("THREE.WebGLRenderer: Attempt to load unsupported compressed texture format in .uploadTexture()"):dt?e.texSubImage3D(i.TEXTURE_2D_ARRAY,at,0,0,0,Et.width,Et.height,st.depth,Kt,qt,Et.data):e.texImage3D(i.TEXTURE_2D_ARRAY,at,Lt,Et.width,Et.height,st.depth,0,Kt,qt,Et.data)}else{dt&&It&&e.texStorage2D(i.TEXTURE_2D,St,Lt,F[0].width,F[0].height);for(let at=0,B=F.length;at<B;at++)Et=F[at],A.format!==Wn?Kt!==null?dt?e.compressedTexSubImage2D(i.TEXTURE_2D,at,0,0,Et.width,Et.height,Kt,Et.data):e.compressedTexImage2D(i.TEXTURE_2D,at,Lt,Et.width,Et.height,0,Et.data):console.warn("THREE.WebGLRenderer: Attempt to load unsupported compressed texture format in .uploadTexture()"):dt?e.texSubImage2D(i.TEXTURE_2D,at,0,0,Et.width,Et.height,Kt,qt,Et.data):e.texImage2D(i.TEXTURE_2D,at,Lt,Et.width,Et.height,0,Kt,qt,Et.data)}else if(A.isDataArrayTexture)dt?(It&&e.texStorage3D(i.TEXTURE_2D_ARRAY,St,Lt,st.width,st.height,st.depth),e.texSubImage3D(i.TEXTURE_2D_ARRAY,0,0,0,0,st.width,st.height,st.depth,Kt,qt,st.data)):e.texImage3D(i.TEXTURE_2D_ARRAY,0,Lt,st.width,st.height,st.depth,0,Kt,qt,st.data);else if(A.isData3DTexture)dt?(It&&e.texStorage3D(i.TEXTURE_3D,St,Lt,st.width,st.height,st.depth),e.texSubImage3D(i.TEXTURE_3D,0,0,0,0,st.width,st.height,st.depth,Kt,qt,st.data)):e.texImage3D(i.TEXTURE_3D,0,Lt,st.width,st.height,st.depth,0,Kt,qt,st.data);else if(A.isFramebufferTexture){if(It)if(dt)e.texStorage2D(i.TEXTURE_2D,St,Lt,st.width,st.height);else{let at=st.width,B=st.height;for(let ft=0;ft<St;ft++)e.texImage2D(i.TEXTURE_2D,ft,Lt,at,B,0,Kt,qt,null),at>>=1,B>>=1}}else if(F.length>0&&he){dt&&It&&e.texStorage2D(i.TEXTURE_2D,St,Lt,F[0].width,F[0].height);for(let at=0,B=F.length;at<B;at++)Et=F[at],dt?e.texSubImage2D(i.TEXTURE_2D,at,0,0,Kt,qt,Et):e.texImage2D(i.TEXTURE_2D,at,Lt,Kt,qt,Et);A.generateMipmaps=!1}else dt?(It&&e.texStorage2D(i.TEXTURE_2D,St,Lt,st.width,st.height),e.texSubImage2D(i.TEXTURE_2D,0,0,0,Kt,qt,st)):e.texImage2D(i.TEXTURE_2D,0,Lt,Kt,qt,st);C(A,he)&&w(lt),Ct.__version=ct.version,A.onUpdate&&A.onUpdate(A)}O.__version=A.version}function Pt(O,A,q){if(A.image.length!==6)return;const lt=it(O,A),rt=A.source;e.bindTexture(i.TEXTURE_CUBE_MAP,O.__webglTexture,i.TEXTURE0+q);const ct=n.get(rt);if(rt.version!==ct.__version||lt===!0){e.activeTexture(i.TEXTURE0+q);const Ct=ve.getPrimaries(ve.workingColorSpace),ut=A.colorSpace===Nn?null:ve.getPrimaries(A.colorSpace),_t=A.colorSpace===Nn||Ct===ut?i.NONE:i.BROWSER_DEFAULT_WEBGL;i.pixelStorei(i.UNPACK_FLIP_Y_WEBGL,A.flipY),i.pixelStorei(i.UNPACK_PREMULTIPLY_ALPHA_WEBGL,A.premultiplyAlpha),i.pixelStorei(i.UNPACK_ALIGNMENT,A.unpackAlignment),i.pixelStorei(i.UNPACK_COLORSPACE_CONVERSION_WEBGL,_t);const Nt=A.isCompressedTexture||A.image[0].isCompressedTexture,Qt=A.image[0]&&A.image[0].isDataTexture,st=[];for(let at=0;at<6;at++)!Nt&&!Qt?st[at]=E(A.image[at],!1,!0,r.maxCubemapSize):st[at]=Qt?A.image[at].image:A.image[at],st[at]=te(A,st[at]);const he=st[0],Kt=_(he)||a,qt=s.convert(A.format,A.colorSpace),Lt=s.convert(A.type),Et=b(A.internalFormat,qt,Lt,A.colorSpace),F=a&&A.isVideoTexture!==!0,dt=ct.__version===void 0||lt===!0;let It=N(A,he,Kt);X(i.TEXTURE_CUBE_MAP,A,Kt);let St;if(Nt){F&&dt&&e.texStorage2D(i.TEXTURE_CUBE_MAP,It,Et,he.width,he.height);for(let at=0;at<6;at++){St=st[at].mipmaps;for(let B=0;B<St.length;B++){const ft=St[B];A.format!==Wn?qt!==null?F?e.compressedTexSubImage2D(i.TEXTURE_CUBE_MAP_POSITIVE_X+at,B,0,0,ft.width,ft.height,qt,ft.data):e.compressedTexImage2D(i.TEXTURE_CUBE_MAP_POSITIVE_X+at,B,Et,ft.width,ft.height,0,ft.data):console.warn("THREE.WebGLRenderer: Attempt to load unsupported compressed texture format in .setTextureCube()"):F?e.texSubImage2D(i.TEXTURE_CUBE_MAP_POSITIVE_X+at,B,0,0,ft.width,ft.height,qt,Lt,ft.data):e.texImage2D(i.TEXTURE_CUBE_MAP_POSITIVE_X+at,B,Et,ft.width,ft.height,0,qt,Lt,ft.data)}}}else{St=A.mipmaps,F&&dt&&(St.length>0&&It++,e.texStorage2D(i.TEXTURE_CUBE_MAP,It,Et,st[0].width,st[0].height));for(let at=0;at<6;at++)if(Qt){F?e.texSubImage2D(i.TEXTURE_CUBE_MAP_POSITIVE_X+at,0,0,0,st[at].width,st[at].height,qt,Lt,st[at].data):e.texImage2D(i.TEXTURE_CUBE_MAP_POSITIVE_X+at,0,Et,st[at].width,st[at].height,0,qt,Lt,st[at].data);for(let B=0;B<St.length;B++){const yt=St[B].image[at].image;F?e.texSubImage2D(i.TEXTURE_CUBE_MAP_POSITIVE_X+at,B+1,0,0,yt.width,yt.height,qt,Lt,yt.data):e.texImage2D(i.TEXTURE_CUBE_MAP_POSITIVE_X+at,B+1,Et,yt.width,yt.height,0,qt,Lt,yt.data)}}else{F?e.texSubImage2D(i.TEXTURE_CUBE_MAP_POSITIVE_X+at,0,0,0,qt,Lt,st[at]):e.texImage2D(i.TEXTURE_CUBE_MAP_POSITIVE_X+at,0,Et,qt,Lt,st[at]);for(let B=0;B<St.length;B++){const ft=St[B];F?e.texSubImage2D(i.TEXTURE_CUBE_MAP_POSITIVE_X+at,B+1,0,0,qt,Lt,ft.image[at]):e.texImage2D(i.TEXTURE_CUBE_MAP_POSITIVE_X+at,B+1,Et,qt,Lt,ft.image[at])}}}C(A,Kt)&&w(i.TEXTURE_CUBE_MAP),ct.__version=rt.version,A.onUpdate&&A.onUpdate(A)}O.__version=A.version}function Mt(O,A,q,lt,rt,ct){const Ct=s.convert(q.format,q.colorSpace),ut=s.convert(q.type),_t=b(q.internalFormat,Ct,ut,q.colorSpace);if(!n.get(A).__hasExternalTextures){const Qt=Math.max(1,A.width>>ct),st=Math.max(1,A.height>>ct);rt===i.TEXTURE_3D||rt===i.TEXTURE_2D_ARRAY?e.texImage3D(rt,ct,_t,Qt,st,A.depth,0,Ct,ut,null):e.texImage2D(rt,ct,_t,Qt,st,0,Ct,ut,null)}e.bindFramebuffer(i.FRAMEBUFFER,O),bt(A)?c.framebufferTexture2DMultisampleEXT(i.FRAMEBUFFER,lt,rt,n.get(q).__webglTexture,0,kt(A)):(rt===i.TEXTURE_2D||rt>=i.TEXTURE_CUBE_MAP_POSITIVE_X&&rt<=i.TEXTURE_CUBE_MAP_NEGATIVE_Z)&&i.framebufferTexture2D(i.FRAMEBUFFER,lt,rt,n.get(q).__webglTexture,ct),e.bindFramebuffer(i.FRAMEBUFFER,null)}function Gt(O,A,q){if(i.bindRenderbuffer(i.RENDERBUFFER,O),A.depthBuffer&&!A.stencilBuffer){let lt=a===!0?i.DEPTH_COMPONENT24:i.DEPTH_COMPONENT16;if(q||bt(A)){const rt=A.depthTexture;rt&&rt.isDepthTexture&&(rt.type===Ci?lt=i.DEPTH_COMPONENT32F:rt.type===Ri&&(lt=i.DEPTH_COMPONENT24));const ct=kt(A);bt(A)?c.renderbufferStorageMultisampleEXT(i.RENDERBUFFER,ct,lt,A.width,A.height):i.renderbufferStorageMultisample(i.RENDERBUFFER,ct,lt,A.width,A.height)}else i.renderbufferStorage(i.RENDERBUFFER,lt,A.width,A.height);i.framebufferRenderbuffer(i.FRAMEBUFFER,i.DEPTH_ATTACHMENT,i.RENDERBUFFER,O)}else if(A.depthBuffer&&A.stencilBuffer){const lt=kt(A);q&&bt(A)===!1?i.renderbufferStorageMultisample(i.RENDERBUFFER,lt,i.DEPTH24_STENCIL8,A.width,A.height):bt(A)?c.renderbufferStorageMultisampleEXT(i.RENDERBUFFER,lt,i.DEPTH24_STENCIL8,A.width,A.height):i.renderbufferStorage(i.RENDERBUFFER,i.DEPTH_STENCIL,A.width,A.height),i.framebufferRenderbuffer(i.FRAMEBUFFER,i.DEPTH_STENCIL_ATTACHMENT,i.RENDERBUFFER,O)}else{const lt=A.isWebGLMultipleRenderTargets===!0?A.texture:[A.texture];for(let rt=0;rt<lt.length;rt++){const ct=lt[rt],Ct=s.convert(ct.format,ct.colorSpace),ut=s.convert(ct.type),_t=b(ct.internalFormat,Ct,ut,ct.colorSpace),Nt=kt(A);q&&bt(A)===!1?i.renderbufferStorageMultisample(i.RENDERBUFFER,Nt,_t,A.width,A.height):bt(A)?c.renderbufferStorageMultisampleEXT(i.RENDERBUFFER,Nt,_t,A.width,A.height):i.renderbufferStorage(i.RENDERBUFFER,_t,A.width,A.height)}}i.bindRenderbuffer(i.RENDERBUFFER,null)}function Ft(O,A){if(A&&A.isWebGLCubeRenderTarget)throw new Error("Depth Texture with cube render targets is not supported");if(e.bindFramebuffer(i.FRAMEBUFFER,O),!(A.depthTexture&&A.depthTexture.isDepthTexture))throw new Error("renderTarget.depthTexture must be an instance of THREE.DepthTexture");(!n.get(A.depthTexture).__webglTexture||A.depthTexture.image.width!==A.width||A.depthTexture.image.height!==A.height)&&(A.depthTexture.image.width=A.width,A.depthTexture.image.height=A.height,A.depthTexture.needsUpdate=!0),K(A.depthTexture,0);const lt=n.get(A.depthTexture).__webglTexture,rt=kt(A);if(A.depthTexture.format===ar)bt(A)?c.framebufferTexture2DMultisampleEXT(i.FRAMEBUFFER,i.DEPTH_ATTACHMENT,i.TEXTURE_2D,lt,0,rt):i.framebufferTexture2D(i.FRAMEBUFFER,i.DEPTH_ATTACHMENT,i.TEXTURE_2D,lt,0);else if(A.depthTexture.format===os)bt(A)?c.framebufferTexture2DMultisampleEXT(i.FRAMEBUFFER,i.DEPTH_STENCIL_ATTACHMENT,i.TEXTURE_2D,lt,0,rt):i.framebufferTexture2D(i.FRAMEBUFFER,i.DEPTH_STENCIL_ATTACHMENT,i.TEXTURE_2D,lt,0);else throw new Error("Unknown depthTexture format")}function wt(O){const A=n.get(O),q=O.isWebGLCubeRenderTarget===!0;if(O.depthTexture&&!A.__autoAllocateDepthBuffer){if(q)throw new Error("target.depthTexture not supported in Cube render targets");Ft(A.__webglFramebuffer,O)}else if(q){A.__webglDepthbuffer=[];for(let lt=0;lt<6;lt++)e.bindFramebuffer(i.FRAMEBUFFER,A.__webglFramebuffer[lt]),A.__webglDepthbuffer[lt]=i.createRenderbuffer(),Gt(A.__webglDepthbuffer[lt],O,!1)}else e.bindFramebuffer(i.FRAMEBUFFER,A.__webglFramebuffer),A.__webglDepthbuffer=i.createRenderbuffer(),Gt(A.__webglDepthbuffer,O,!1);e.bindFramebuffer(i.FRAMEBUFFER,null)}function Wt(O,A,q){const lt=n.get(O);A!==void 0&&Mt(lt.__webglFramebuffer,O,O.texture,i.COLOR_ATTACHMENT0,i.TEXTURE_2D,0),q!==void 0&&wt(O)}function W(O){const A=O.texture,q=n.get(O),lt=n.get(A);O.addEventListener("dispose",M),O.isWebGLMultipleRenderTargets!==!0&&(lt.__webglTexture===void 0&&(lt.__webglTexture=i.createTexture()),lt.__version=A.version,o.memory.textures++);const rt=O.isWebGLCubeRenderTarget===!0,ct=O.isWebGLMultipleRenderTargets===!0,Ct=_(O)||a;if(rt){q.__webglFramebuffer=[];for(let ut=0;ut<6;ut++)if(a&&A.mipmaps&&A.mipmaps.length>0){q.__webglFramebuffer[ut]=[];for(let _t=0;_t<A.mipmaps.length;_t++)q.__webglFramebuffer[ut][_t]=i.createFramebuffer()}else q.__webglFramebuffer[ut]=i.createFramebuffer()}else{if(a&&A.mipmaps&&A.mipmaps.length>0){q.__webglFramebuffer=[];for(let ut=0;ut<A.mipmaps.length;ut++)q.__webglFramebuffer[ut]=i.createFramebuffer()}else q.__webglFramebuffer=i.createFramebuffer();if(ct)if(r.drawBuffers){const ut=O.texture;for(let _t=0,Nt=ut.length;_t<Nt;_t++){const Qt=n.get(ut[_t]);Qt.__webglTexture===void 0&&(Qt.__webglTexture=i.createTexture(),o.memory.textures++)}}else console.warn("THREE.WebGLRenderer: WebGLMultipleRenderTargets can only be used with WebGL2 or WEBGL_draw_buffers extension.");if(a&&O.samples>0&&bt(O)===!1){const ut=ct?A:[A];q.__webglMultisampledFramebuffer=i.createFramebuffer(),q.__webglColorRenderbuffer=[],e.bindFramebuffer(i.FRAMEBUFFER,q.__webglMultisampledFramebuffer);for(let _t=0;_t<ut.length;_t++){const Nt=ut[_t];q.__webglColorRenderbuffer[_t]=i.createRenderbuffer(),i.bindRenderbuffer(i.RENDERBUFFER,q.__webglColorRenderbuffer[_t]);const Qt=s.convert(Nt.format,Nt.colorSpace),st=s.convert(Nt.type),he=b(Nt.internalFormat,Qt,st,Nt.colorSpace,O.isXRRenderTarget===!0),Kt=kt(O);i.renderbufferStorageMultisample(i.RENDERBUFFER,Kt,he,O.width,O.height),i.framebufferRenderbuffer(i.FRAMEBUFFER,i.COLOR_ATTACHMENT0+_t,i.RENDERBUFFER,q.__webglColorRenderbuffer[_t])}i.bindRenderbuffer(i.RENDERBUFFER,null),O.depthBuffer&&(q.__webglDepthRenderbuffer=i.createRenderbuffer(),Gt(q.__webglDepthRenderbuffer,O,!0)),e.bindFramebuffer(i.FRAMEBUFFER,null)}}if(rt){e.bindTexture(i.TEXTURE_CUBE_MAP,lt.__webglTexture),X(i.TEXTURE_CUBE_MAP,A,Ct);for(let ut=0;ut<6;ut++)if(a&&A.mipmaps&&A.mipmaps.length>0)for(let _t=0;_t<A.mipmaps.length;_t++)Mt(q.__webglFramebuffer[ut][_t],O,A,i.COLOR_ATTACHMENT0,i.TEXTURE_CUBE_MAP_POSITIVE_X+ut,_t);else Mt(q.__webglFramebuffer[ut],O,A,i.COLOR_ATTACHMENT0,i.TEXTURE_CUBE_MAP_POSITIVE_X+ut,0);C(A,Ct)&&w(i.TEXTURE_CUBE_MAP),e.unbindTexture()}else if(ct){const ut=O.texture;for(let _t=0,Nt=ut.length;_t<Nt;_t++){const Qt=ut[_t],st=n.get(Qt);e.bindTexture(i.TEXTURE_2D,st.__webglTexture),X(i.TEXTURE_2D,Qt,Ct),Mt(q.__webglFramebuffer,O,Qt,i.COLOR_ATTACHMENT0+_t,i.TEXTURE_2D,0),C(Qt,Ct)&&w(i.TEXTURE_2D)}e.unbindTexture()}else{let ut=i.TEXTURE_2D;if((O.isWebGL3DRenderTarget||O.isWebGLArrayRenderTarget)&&(a?ut=O.isWebGL3DRenderTarget?i.TEXTURE_3D:i.TEXTURE_2D_ARRAY:console.error("THREE.WebGLTextures: THREE.Data3DTexture and THREE.DataArrayTexture only supported with WebGL2.")),e.bindTexture(ut,lt.__webglTexture),X(ut,A,Ct),a&&A.mipmaps&&A.mipmaps.length>0)for(let _t=0;_t<A.mipmaps.length;_t++)Mt(q.__webglFramebuffer[_t],O,A,i.COLOR_ATTACHMENT0,ut,_t);else Mt(q.__webglFramebuffer,O,A,i.COLOR_ATTACHMENT0,ut,0);C(A,Ct)&&w(ut),e.unbindTexture()}O.depthBuffer&&wt(O)}function pe(O){const A=_(O)||a,q=O.isWebGLMultipleRenderTargets===!0?O.texture:[O.texture];for(let lt=0,rt=q.length;lt<rt;lt++){const ct=q[lt];if(C(ct,A)){const Ct=O.isWebGLCubeRenderTarget?i.TEXTURE_CUBE_MAP:i.TEXTURE_2D,ut=n.get(ct).__webglTexture;e.bindTexture(Ct,ut),w(Ct),e.unbindTexture()}}}function Rt(O){if(a&&O.samples>0&&bt(O)===!1){const A=O.isWebGLMultipleRenderTargets?O.texture:[O.texture],q=O.width,lt=O.height;let rt=i.COLOR_BUFFER_BIT;const ct=[],Ct=O.stencilBuffer?i.DEPTH_STENCIL_ATTACHMENT:i.DEPTH_ATTACHMENT,ut=n.get(O),_t=O.isWebGLMultipleRenderTargets===!0;if(_t)for(let Nt=0;Nt<A.length;Nt++)e.bindFramebuffer(i.FRAMEBUFFER,ut.__webglMultisampledFramebuffer),i.framebufferRenderbuffer(i.FRAMEBUFFER,i.COLOR_ATTACHMENT0+Nt,i.RENDERBUFFER,null),e.bindFramebuffer(i.FRAMEBUFFER,ut.__webglFramebuffer),i.framebufferTexture2D(i.DRAW_FRAMEBUFFER,i.COLOR_ATTACHMENT0+Nt,i.TEXTURE_2D,null,0);e.bindFramebuffer(i.READ_FRAMEBUFFER,ut.__webglMultisampledFramebuffer),e.bindFramebuffer(i.DRAW_FRAMEBUFFER,ut.__webglFramebuffer);for(let Nt=0;Nt<A.length;Nt++){ct.push(i.COLOR_ATTACHMENT0+Nt),O.depthBuffer&&ct.push(Ct);const Qt=ut.__ignoreDepthValues!==void 0?ut.__ignoreDepthValues:!1;if(Qt===!1&&(O.depthBuffer&&(rt|=i.DEPTH_BUFFER_BIT),O.stencilBuffer&&(rt|=i.STENCIL_BUFFER_BIT)),_t&&i.framebufferRenderbuffer(i.READ_FRAMEBUFFER,i.COLOR_ATTACHMENT0,i.RENDERBUFFER,ut.__webglColorRenderbuffer[Nt]),Qt===!0&&(i.invalidateFramebuffer(i.READ_FRAMEBUFFER,[Ct]),i.invalidateFramebuffer(i.DRAW_FRAMEBUFFER,[Ct])),_t){const st=n.get(A[Nt]).__webglTexture;i.framebufferTexture2D(i.DRAW_FRAMEBUFFER,i.COLOR_ATTACHMENT0,i.TEXTURE_2D,st,0)}i.blitFramebuffer(0,0,q,lt,0,0,q,lt,rt,i.NEAREST),h&&i.invalidateFramebuffer(i.READ_FRAMEBUFFER,ct)}if(e.bindFramebuffer(i.READ_FRAMEBUFFER,null),e.bindFramebuffer(i.DRAW_FRAMEBUFFER,null),_t)for(let Nt=0;Nt<A.length;Nt++){e.bindFramebuffer(i.FRAMEBUFFER,ut.__webglMultisampledFramebuffer),i.framebufferRenderbuffer(i.FRAMEBUFFER,i.COLOR_ATTACHMENT0+Nt,i.RENDERBUFFER,ut.__webglColorRenderbuffer[Nt]);const Qt=n.get(A[Nt]).__webglTexture;e.bindFramebuffer(i.FRAMEBUFFER,ut.__webglFramebuffer),i.framebufferTexture2D(i.DRAW_FRAMEBUFFER,i.COLOR_ATTACHMENT0+Nt,i.TEXTURE_2D,Qt,0)}e.bindFramebuffer(i.DRAW_FRAMEBUFFER,ut.__webglMultisampledFramebuffer)}}function kt(O){return Math.min(r.maxSamples,O.samples)}function bt(O){const A=n.get(O);return a&&O.samples>0&&t.has("WEBGL_multisampled_render_to_texture")===!0&&A.__useRenderToTexture!==!1}function ye(O){const A=o.render.frame;u.get(O)!==A&&(u.set(O,A),O.update())}function te(O,A){const q=O.colorSpace,lt=O.format,rt=O.type;return O.isCompressedTexture===!0||O.isVideoTexture===!0||O.format===ul||q!==di&&q!==Nn&&(ve.getTransfer(q)===we?a===!1?t.has("EXT_sRGB")===!0&&lt===Wn?(O.format=ul,O.minFilter=Pn,O.generateMipmaps=!1):A=sp.sRGBToLinear(A):(lt!==Wn||rt!==Ni)&&console.warn("THREE.WebGLTextures: sRGB encoded textures have to use RGBAFormat and UnsignedByteType."):console.error("THREE.WebGLTextures: Unsupported texture color space:",q)),A}this.allocateTextureUnit=V,this.resetTextureUnits=R,this.setTexture2D=K,this.setTexture2DArray=Z,this.setTexture3D=J,this.setTextureCube=et,this.rebindTextures=Wt,this.setupRenderTarget=W,this.updateRenderTargetMipmap=pe,this.updateMultisampleRenderTarget=Rt,this.setupDepthRenderbuffer=wt,this.setupFrameBufferTexture=Mt,this.useMultisampledRTT=bt}function Tx(i,t,e){const n=e.isWebGL2;function r(s,o=Nn){let a;const c=ve.getTransfer(o);if(s===Ni)return i.UNSIGNED_BYTE;if(s===$f)return i.UNSIGNED_SHORT_4_4_4_4;if(s===Kf)return i.UNSIGNED_SHORT_5_5_5_1;if(s===r_)return i.BYTE;if(s===s_)return i.SHORT;if(s===Fl)return i.UNSIGNED_SHORT;if(s===Yf)return i.INT;if(s===Ri)return i.UNSIGNED_INT;if(s===Ci)return i.FLOAT;if(s===eo)return n?i.HALF_FLOAT:(a=t.get("OES_texture_half_float"),a!==null?a.HALF_FLOAT_OES:null);if(s===o_)return i.ALPHA;if(s===Wn)return i.RGBA;if(s===a_)return i.LUMINANCE;if(s===c_)return i.LUMINANCE_ALPHA;if(s===ar)return i.DEPTH_COMPONENT;if(s===os)return i.DEPTH_STENCIL;if(s===ul)return a=t.get("EXT_sRGB"),a!==null?a.SRGB_ALPHA_EXT:null;if(s===l_)return i.RED;if(s===Qf)return i.RED_INTEGER;if(s===h_)return i.RG;if(s===Zf)return i.RG_INTEGER;if(s===Jf)return i.RGBA_INTEGER;if(s===fc||s===pc||s===mc||s===gc)if(c===we)if(a=t.get("WEBGL_compressed_texture_s3tc_srgb"),a!==null){if(s===fc)return a.COMPRESSED_SRGB_S3TC_DXT1_EXT;if(s===pc)return a.COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;if(s===mc)return a.COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;if(s===gc)return a.COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT}else return null;else if(a=t.get("WEBGL_compressed_texture_s3tc"),a!==null){if(s===fc)return a.COMPRESSED_RGB_S3TC_DXT1_EXT;if(s===pc)return a.COMPRESSED_RGBA_S3TC_DXT1_EXT;if(s===mc)return a.COMPRESSED_RGBA_S3TC_DXT3_EXT;if(s===gc)return a.COMPRESSED_RGBA_S3TC_DXT5_EXT}else return null;if(s===uu||s===du||s===fu||s===pu)if(a=t.get("WEBGL_compressed_texture_pvrtc"),a!==null){if(s===uu)return a.COMPRESSED_RGB_PVRTC_4BPPV1_IMG;if(s===du)return a.COMPRESSED_RGB_PVRTC_2BPPV1_IMG;if(s===fu)return a.COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;if(s===pu)return a.COMPRESSED_RGBA_PVRTC_2BPPV1_IMG}else return null;if(s===tp)return a=t.get("WEBGL_compressed_texture_etc1"),a!==null?a.COMPRESSED_RGB_ETC1_WEBGL:null;if(s===mu||s===gu)if(a=t.get("WEBGL_compressed_texture_etc"),a!==null){if(s===mu)return c===we?a.COMPRESSED_SRGB8_ETC2:a.COMPRESSED_RGB8_ETC2;if(s===gu)return c===we?a.COMPRESSED_SRGB8_ALPHA8_ETC2_EAC:a.COMPRESSED_RGBA8_ETC2_EAC}else return null;if(s===_u||s===vu||s===yu||s===Eu||s===xu||s===Su||s===Tu||s===Mu||s===wu||s===bu||s===Au||s===Ru||s===Cu||s===Iu)if(a=t.get("WEBGL_compressed_texture_astc"),a!==null){if(s===_u)return c===we?a.COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR:a.COMPRESSED_RGBA_ASTC_4x4_KHR;if(s===vu)return c===we?a.COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR:a.COMPRESSED_RGBA_ASTC_5x4_KHR;if(s===yu)return c===we?a.COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR:a.COMPRESSED_RGBA_ASTC_5x5_KHR;if(s===Eu)return c===we?a.COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR:a.COMPRESSED_RGBA_ASTC_6x5_KHR;if(s===xu)return c===we?a.COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR:a.COMPRESSED_RGBA_ASTC_6x6_KHR;if(s===Su)return c===we?a.COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR:a.COMPRESSED_RGBA_ASTC_8x5_KHR;if(s===Tu)return c===we?a.COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR:a.COMPRESSED_RGBA_ASTC_8x6_KHR;if(s===Mu)return c===we?a.COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR:a.COMPRESSED_RGBA_ASTC_8x8_KHR;if(s===wu)return c===we?a.COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR:a.COMPRESSED_RGBA_ASTC_10x5_KHR;if(s===bu)return c===we?a.COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR:a.COMPRESSED_RGBA_ASTC_10x6_KHR;if(s===Au)return c===we?a.COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR:a.COMPRESSED_RGBA_ASTC_10x8_KHR;if(s===Ru)return c===we?a.COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR:a.COMPRESSED_RGBA_ASTC_10x10_KHR;if(s===Cu)return c===we?a.COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR:a.COMPRESSED_RGBA_ASTC_12x10_KHR;if(s===Iu)return c===we?a.COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR:a.COMPRESSED_RGBA_ASTC_12x12_KHR}else return null;if(s===_c||s===Pu||s===Du)if(a=t.get("EXT_texture_compression_bptc"),a!==null){if(s===_c)return c===we?a.COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT:a.COMPRESSED_RGBA_BPTC_UNORM_EXT;if(s===Pu)return a.COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT;if(s===Du)return a.COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_EXT}else return null;if(s===u_||s===Lu||s===Nu||s===Uu)if(a=t.get("EXT_texture_compression_rgtc"),a!==null){if(s===_c)return a.COMPRESSED_RED_RGTC1_EXT;if(s===Lu)return a.COMPRESSED_SIGNED_RED_RGTC1_EXT;if(s===Nu)return a.COMPRESSED_RED_GREEN_RGTC2_EXT;if(s===Uu)return a.COMPRESSED_SIGNED_RED_GREEN_RGTC2_EXT}else return null;return s===or?n?i.UNSIGNED_INT_24_8:(a=t.get("WEBGL_depth_texture"),a!==null?a.UNSIGNED_INT_24_8_WEBGL:null):i[s]!==void 0?i[s]:null}return{convert:r}}class Mx extends Ln{constructor(t=[]){super(),this.isArrayCamera=!0,this.cameras=t}}class rr extends je{constructor(){super(),this.isGroup=!0,this.type="Group"}}const wx={type:"move"};class Hc{constructor(){this._targetRay=null,this._grip=null,this._hand=null}getHandSpace(){return this._hand===null&&(this._hand=new rr,this._hand.matrixAutoUpdate=!1,this._hand.visible=!1,this._hand.joints={},this._hand.inputState={pinching:!1}),this._hand}getTargetRaySpace(){return this._targetRay===null&&(this._targetRay=new rr,this._targetRay.matrixAutoUpdate=!1,this._targetRay.visible=!1,this._targetRay.hasLinearVelocity=!1,this._targetRay.linearVelocity=new k,this._targetRay.hasAngularVelocity=!1,this._targetRay.angularVelocity=new k),this._targetRay}getGripSpace(){return this._grip===null&&(this._grip=new rr,this._grip.matrixAutoUpdate=!1,this._grip.visible=!1,this._grip.hasLinearVelocity=!1,this._grip.linearVelocity=new k,this._grip.hasAngularVelocity=!1,this._grip.angularVelocity=new k),this._grip}dispatchEvent(t){return this._targetRay!==null&&this._targetRay.dispatchEvent(t),this._grip!==null&&this._grip.dispatchEvent(t),this._hand!==null&&this._hand.dispatchEvent(t),this}connect(t){if(t&&t.hand){const e=this._hand;if(e)for(const n of t.hand.values())this._getHandJoint(e,n)}return this.dispatchEvent({type:"connected",data:t}),this}disconnect(t){return this.dispatchEvent({type:"disconnected",data:t}),this._targetRay!==null&&(this._targetRay.visible=!1),this._grip!==null&&(this._grip.visible=!1),this._hand!==null&&(this._hand.visible=!1),this}update(t,e,n){let r=null,s=null,o=null;const a=this._targetRay,c=this._grip,h=this._hand;if(t&&e.session.visibilityState!=="visible-blurred"){if(h&&t.hand){o=!0;for(const E of t.hand.values()){const _=e.getJointPose(E,n),m=this._getHandJoint(h,E);_!==null&&(m.matrix.fromArray(_.transform.matrix),m.matrix.decompose(m.position,m.rotation,m.scale),m.matrixWorldNeedsUpdate=!0,m.jointRadius=_.radius),m.visible=_!==null}const u=h.joints["index-finger-tip"],d=h.joints["thumb-tip"],f=u.position.distanceTo(d.position),p=.02,y=.005;h.inputState.pinching&&f>p+y?(h.inputState.pinching=!1,this.dispatchEvent({type:"pinchend",handedness:t.handedness,target:this})):!h.inputState.pinching&&f<=p-y&&(h.inputState.pinching=!0,this.dispatchEvent({type:"pinchstart",handedness:t.handedness,target:this}))}else c!==null&&t.gripSpace&&(s=e.getPose(t.gripSpace,n),s!==null&&(c.matrix.fromArray(s.transform.matrix),c.matrix.decompose(c.position,c.rotation,c.scale),c.matrixWorldNeedsUpdate=!0,s.linearVelocity?(c.hasLinearVelocity=!0,c.linearVelocity.copy(s.linearVelocity)):c.hasLinearVelocity=!1,s.angularVelocity?(c.hasAngularVelocity=!0,c.angularVelocity.copy(s.angularVelocity)):c.hasAngularVelocity=!1));a!==null&&(r=e.getPose(t.targetRaySpace,n),r===null&&s!==null&&(r=s),r!==null&&(a.matrix.fromArray(r.transform.matrix),a.matrix.decompose(a.position,a.rotation,a.scale),a.matrixWorldNeedsUpdate=!0,r.linearVelocity?(a.hasLinearVelocity=!0,a.linearVelocity.copy(r.linearVelocity)):a.hasLinearVelocity=!1,r.angularVelocity?(a.hasAngularVelocity=!0,a.angularVelocity.copy(r.angularVelocity)):a.hasAngularVelocity=!1,this.dispatchEvent(wx)))}return a!==null&&(a.visible=r!==null),c!==null&&(c.visible=s!==null),h!==null&&(h.visible=o!==null),this}_getHandJoint(t,e){if(t.joints[e.jointName]===void 0){const n=new rr;n.matrixAutoUpdate=!1,n.visible=!1,t.joints[e.jointName]=n,t.add(n)}return t.joints[e.jointName]}}class bx extends gr{constructor(t,e){super();const n=this;let r=null,s=1,o=null,a="local-floor",c=1,h=null,u=null,d=null,f=null,p=null,y=null;const E=e.getContextAttributes();let _=null,m=null;const C=[],w=[],b=new Vt;let N=null;const U=new Ln;U.layers.enable(1),U.viewport=new Xe;const T=new Ln;T.layers.enable(2),T.viewport=new Xe;const M=[U,T],v=new Mx;v.layers.enable(1),v.layers.enable(2);let S=null,P=null;this.cameraAutoUpdate=!0,this.enabled=!1,this.isPresenting=!1,this.getController=function(X){let it=C[X];return it===void 0&&(it=new Hc,C[X]=it),it.getTargetRaySpace()},this.getControllerGrip=function(X){let it=C[X];return it===void 0&&(it=new Hc,C[X]=it),it.getGripSpace()},this.getHand=function(X){let it=C[X];return it===void 0&&(it=new Hc,C[X]=it),it.getHandSpace()};function L(X){const it=w.indexOf(X.inputSource);if(it===-1)return;const mt=C[it];mt!==void 0&&(mt.update(X.inputSource,X.frame,h||o),mt.dispatchEvent({type:X.type,data:X.inputSource}))}function R(){r.removeEventListener("select",L),r.removeEventListener("selectstart",L),r.removeEventListener("selectend",L),r.removeEventListener("squeeze",L),r.removeEventListener("squeezestart",L),r.removeEventListener("squeezeend",L),r.removeEventListener("end",R),r.removeEventListener("inputsourceschange",V);for(let X=0;X<C.length;X++){const it=w[X];it!==null&&(w[X]=null,C[X].disconnect(it))}S=null,P=null,t.setRenderTarget(_),p=null,f=null,d=null,r=null,m=null,ht.stop(),n.isPresenting=!1,t.setPixelRatio(N),t.setSize(b.width,b.height,!1),n.dispatchEvent({type:"sessionend"})}this.setFramebufferScaleFactor=function(X){s=X,n.isPresenting===!0&&console.warn("THREE.WebXRManager: Cannot change framebuffer scale while presenting.")},this.setReferenceSpaceType=function(X){a=X,n.isPresenting===!0&&console.warn("THREE.WebXRManager: Cannot change reference space type while presenting.")},this.getReferenceSpace=function(){return h||o},this.setReferenceSpace=function(X){h=X},this.getBaseLayer=function(){return f!==null?f:p},this.getBinding=function(){return d},this.getFrame=function(){return y},this.getSession=function(){return r},this.setSession=async function(X){if(r=X,r!==null){if(_=t.getRenderTarget(),r.addEventListener("select",L),r.addEventListener("selectstart",L),r.addEventListener("selectend",L),r.addEventListener("squeeze",L),r.addEventListener("squeezestart",L),r.addEventListener("squeezeend",L),r.addEventListener("end",R),r.addEventListener("inputsourceschange",V),E.xrCompatible!==!0&&await e.makeXRCompatible(),N=t.getPixelRatio(),t.getSize(b),r.renderState.layers===void 0||t.capabilities.isWebGL2===!1){const it={antialias:r.renderState.layers===void 0?E.antialias:!0,alpha:!0,depth:E.depth,stencil:E.stencil,framebufferScaleFactor:s};p=new XRWebGLLayer(r,e,it),r.updateRenderState({baseLayer:p}),t.setPixelRatio(1),t.setSize(p.framebufferWidth,p.framebufferHeight,!1),m=new hr(p.framebufferWidth,p.framebufferHeight,{format:Wn,type:Ni,colorSpace:t.outputColorSpace,stencilBuffer:E.stencil})}else{let it=null,mt=null,Pt=null;E.depth&&(Pt=E.stencil?e.DEPTH24_STENCIL8:e.DEPTH_COMPONENT24,it=E.stencil?os:ar,mt=E.stencil?or:Ri);const Mt={colorFormat:e.RGBA8,depthFormat:Pt,scaleFactor:s};d=new XRWebGLBinding(r,e),f=d.createProjectionLayer(Mt),r.updateRenderState({layers:[f]}),t.setPixelRatio(1),t.setSize(f.textureWidth,f.textureHeight,!1),m=new hr(f.textureWidth,f.textureHeight,{format:Wn,type:Ni,depthTexture:new gp(f.textureWidth,f.textureHeight,mt,void 0,void 0,void 0,void 0,void 0,void 0,it),stencilBuffer:E.stencil,colorSpace:t.outputColorSpace,samples:E.antialias?4:0});const Gt=t.properties.get(m);Gt.__ignoreDepthValues=f.ignoreDepthValues}m.isXRRenderTarget=!0,this.setFoveation(c),h=null,o=await r.requestReferenceSpace(a),ht.setContext(r),ht.start(),n.isPresenting=!0,n.dispatchEvent({type:"sessionstart"})}},this.getEnvironmentBlendMode=function(){if(r!==null)return r.environmentBlendMode};function V(X){for(let it=0;it<X.removed.length;it++){const mt=X.removed[it],Pt=w.indexOf(mt);Pt>=0&&(w[Pt]=null,C[Pt].disconnect(mt))}for(let it=0;it<X.added.length;it++){const mt=X.added[it];let Pt=w.indexOf(mt);if(Pt===-1){for(let Gt=0;Gt<C.length;Gt++)if(Gt>=w.length){w.push(mt),Pt=Gt;break}else if(w[Gt]===null){w[Gt]=mt,Pt=Gt;break}if(Pt===-1)break}const Mt=C[Pt];Mt&&Mt.connect(mt)}}const G=new k,K=new k;function Z(X,it,mt){G.setFromMatrixPosition(it.matrixWorld),K.setFromMatrixPosition(mt.matrixWorld);const Pt=G.distanceTo(K),Mt=it.projectionMatrix.elements,Gt=mt.projectionMatrix.elements,Ft=Mt[14]/(Mt[10]-1),wt=Mt[14]/(Mt[10]+1),Wt=(Mt[9]+1)/Mt[5],W=(Mt[9]-1)/Mt[5],pe=(Mt[8]-1)/Mt[0],Rt=(Gt[8]+1)/Gt[0],kt=Ft*pe,bt=Ft*Rt,ye=Pt/(-pe+Rt),te=ye*-pe;it.matrixWorld.decompose(X.position,X.quaternion,X.scale),X.translateX(te),X.translateZ(ye),X.matrixWorld.compose(X.position,X.quaternion,X.scale),X.matrixWorldInverse.copy(X.matrixWorld).invert();const O=Ft+ye,A=wt+ye,q=kt-te,lt=bt+(Pt-te),rt=Wt*wt/A*O,ct=W*wt/A*O;X.projectionMatrix.makePerspective(q,lt,rt,ct,O,A),X.projectionMatrixInverse.copy(X.projectionMatrix).invert()}function J(X,it){it===null?X.matrixWorld.copy(X.matrix):X.matrixWorld.multiplyMatrices(it.matrixWorld,X.matrix),X.matrixWorldInverse.copy(X.matrixWorld).invert()}this.updateCamera=function(X){if(r===null)return;v.near=T.near=U.near=X.near,v.far=T.far=U.far=X.far,(S!==v.near||P!==v.far)&&(r.updateRenderState({depthNear:v.near,depthFar:v.far}),S=v.near,P=v.far);const it=X.parent,mt=v.cameras;J(v,it);for(let Pt=0;Pt<mt.length;Pt++)J(mt[Pt],it);mt.length===2?Z(v,U,T):v.projectionMatrix.copy(U.projectionMatrix),et(X,v,it)};function et(X,it,mt){mt===null?X.matrix.copy(it.matrixWorld):(X.matrix.copy(mt.matrixWorld),X.matrix.invert(),X.matrix.multiply(it.matrixWorld)),X.matrix.decompose(X.position,X.quaternion,X.scale),X.updateMatrixWorld(!0),X.projectionMatrix.copy(it.projectionMatrix),X.projectionMatrixInverse.copy(it.projectionMatrixInverse),X.isPerspectiveCamera&&(X.fov=dl*2*Math.atan(1/X.projectionMatrix.elements[5]),X.zoom=1)}this.getCamera=function(){return v},this.getFoveation=function(){if(!(f===null&&p===null))return c},this.setFoveation=function(X){c=X,f!==null&&(f.fixedFoveation=X),p!==null&&p.fixedFoveation!==void 0&&(p.fixedFoveation=X)};let tt=null;function pt(X,it){if(u=it.getViewerPose(h||o),y=it,u!==null){const mt=u.views;p!==null&&(t.setRenderTargetFramebuffer(m,p.framebuffer),t.setRenderTarget(m));let Pt=!1;mt.length!==v.cameras.length&&(v.cameras.length=0,Pt=!0);for(let Mt=0;Mt<mt.length;Mt++){const Gt=mt[Mt];let Ft=null;if(p!==null)Ft=p.getViewport(Gt);else{const Wt=d.getViewSubImage(f,Gt);Ft=Wt.viewport,Mt===0&&(t.setRenderTargetTextures(m,Wt.colorTexture,f.ignoreDepthValues?void 0:Wt.depthStencilTexture),t.setRenderTarget(m))}let wt=M[Mt];wt===void 0&&(wt=new Ln,wt.layers.enable(Mt),wt.viewport=new Xe,M[Mt]=wt),wt.matrix.fromArray(Gt.transform.matrix),wt.matrix.decompose(wt.position,wt.quaternion,wt.scale),wt.projectionMatrix.fromArray(Gt.projectionMatrix),wt.projectionMatrixInverse.copy(wt.projectionMatrix).invert(),wt.viewport.set(Ft.x,Ft.y,Ft.width,Ft.height),Mt===0&&(v.matrix.copy(wt.matrix),v.matrix.decompose(v.position,v.quaternion,v.scale)),Pt===!0&&v.cameras.push(wt)}}for(let mt=0;mt<C.length;mt++){const Pt=w[mt],Mt=C[mt];Pt!==null&&Mt!==void 0&&Mt.update(Pt,it,h||o)}tt&&tt(X,it),it.detectedPlanes&&n.dispatchEvent({type:"planesdetected",data:it}),y=null}const ht=new pp;ht.setAnimationLoop(pt),this.setAnimationLoop=function(X){tt=X},this.dispose=function(){}}}function Ax(i,t){function e(_,m){_.matrixAutoUpdate===!0&&_.updateMatrix(),m.value.copy(_.matrix)}function n(_,m){m.color.getRGB(_.fogColor.value,up(i)),m.isFog?(_.fogNear.value=m.near,_.fogFar.value=m.far):m.isFogExp2&&(_.fogDensity.value=m.density)}function r(_,m,C,w,b){m.isMeshBasicMaterial||m.isMeshLambertMaterial?s(_,m):m.isMeshToonMaterial?(s(_,m),d(_,m)):m.isMeshPhongMaterial?(s(_,m),u(_,m)):m.isMeshStandardMaterial?(s(_,m),f(_,m),m.isMeshPhysicalMaterial&&p(_,m,b)):m.isMeshMatcapMaterial?(s(_,m),y(_,m)):m.isMeshDepthMaterial?s(_,m):m.isMeshDistanceMaterial?(s(_,m),E(_,m)):m.isMeshNormalMaterial?s(_,m):m.isLineBasicMaterial?(o(_,m),m.isLineDashedMaterial&&a(_,m)):m.isPointsMaterial?c(_,m,C,w):m.isSpriteMaterial?h(_,m):m.isShadowMaterial?(_.color.value.copy(m.color),_.opacity.value=m.opacity):m.isShaderMaterial&&(m.uniformsNeedUpdate=!1)}function s(_,m){_.opacity.value=m.opacity,m.color&&_.diffuse.value.copy(m.color),m.emissive&&_.emissive.value.copy(m.emissive).multiplyScalar(m.emissiveIntensity),m.map&&(_.map.value=m.map,e(m.map,_.mapTransform)),m.alphaMap&&(_.alphaMap.value=m.alphaMap,e(m.alphaMap,_.alphaMapTransform)),m.bumpMap&&(_.bumpMap.value=m.bumpMap,e(m.bumpMap,_.bumpMapTransform),_.bumpScale.value=m.bumpScale,m.side===vn&&(_.bumpScale.value*=-1)),m.normalMap&&(_.normalMap.value=m.normalMap,e(m.normalMap,_.normalMapTransform),_.normalScale.value.copy(m.normalScale),m.side===vn&&_.normalScale.value.negate()),m.displacementMap&&(_.displacementMap.value=m.displacementMap,e(m.displacementMap,_.displacementMapTransform),_.displacementScale.value=m.displacementScale,_.displacementBias.value=m.displacementBias),m.emissiveMap&&(_.emissiveMap.value=m.emissiveMap,e(m.emissiveMap,_.emissiveMapTransform)),m.specularMap&&(_.specularMap.value=m.specularMap,e(m.specularMap,_.specularMapTransform)),m.alphaTest>0&&(_.alphaTest.value=m.alphaTest);const C=t.get(m).envMap;if(C&&(_.envMap.value=C,_.flipEnvMap.value=C.isCubeTexture&&C.isRenderTargetTexture===!1?-1:1,_.reflectivity.value=m.reflectivity,_.ior.value=m.ior,_.refractionRatio.value=m.refractionRatio),m.lightMap){_.lightMap.value=m.lightMap;const w=i._useLegacyLights===!0?Math.PI:1;_.lightMapIntensity.value=m.lightMapIntensity*w,e(m.lightMap,_.lightMapTransform)}m.aoMap&&(_.aoMap.value=m.aoMap,_.aoMapIntensity.value=m.aoMapIntensity,e(m.aoMap,_.aoMapTransform))}function o(_,m){_.diffuse.value.copy(m.color),_.opacity.value=m.opacity,m.map&&(_.map.value=m.map,e(m.map,_.mapTransform))}function a(_,m){_.dashSize.value=m.dashSize,_.totalSize.value=m.dashSize+m.gapSize,_.scale.value=m.scale}function c(_,m,C,w){_.diffuse.value.copy(m.color),_.opacity.value=m.opacity,_.size.value=m.size*C,_.scale.value=w*.5,m.map&&(_.map.value=m.map,e(m.map,_.uvTransform)),m.alphaMap&&(_.alphaMap.value=m.alphaMap,e(m.alphaMap,_.alphaMapTransform)),m.alphaTest>0&&(_.alphaTest.value=m.alphaTest)}function h(_,m){_.diffuse.value.copy(m.color),_.opacity.value=m.opacity,_.rotation.value=m.rotation,m.map&&(_.map.value=m.map,e(m.map,_.mapTransform)),m.alphaMap&&(_.alphaMap.value=m.alphaMap,e(m.alphaMap,_.alphaMapTransform)),m.alphaTest>0&&(_.alphaTest.value=m.alphaTest)}function u(_,m){_.specular.value.copy(m.specular),_.shininess.value=Math.max(m.shininess,1e-4)}function d(_,m){m.gradientMap&&(_.gradientMap.value=m.gradientMap)}function f(_,m){_.metalness.value=m.metalness,m.metalnessMap&&(_.metalnessMap.value=m.metalnessMap,e(m.metalnessMap,_.metalnessMapTransform)),_.roughness.value=m.roughness,m.roughnessMap&&(_.roughnessMap.value=m.roughnessMap,e(m.roughnessMap,_.roughnessMapTransform)),t.get(m).envMap&&(_.envMapIntensity.value=m.envMapIntensity)}function p(_,m,C){_.ior.value=m.ior,m.sheen>0&&(_.sheenColor.value.copy(m.sheenColor).multiplyScalar(m.sheen),_.sheenRoughness.value=m.sheenRoughness,m.sheenColorMap&&(_.sheenColorMap.value=m.sheenColorMap,e(m.sheenColorMap,_.sheenColorMapTransform)),m.sheenRoughnessMap&&(_.sheenRoughnessMap.value=m.sheenRoughnessMap,e(m.sheenRoughnessMap,_.sheenRoughnessMapTransform))),m.clearcoat>0&&(_.clearcoat.value=m.clearcoat,_.clearcoatRoughness.value=m.clearcoatRoughness,m.clearcoatMap&&(_.clearcoatMap.value=m.clearcoatMap,e(m.clearcoatMap,_.clearcoatMapTransform)),m.clearcoatRoughnessMap&&(_.clearcoatRoughnessMap.value=m.clearcoatRoughnessMap,e(m.clearcoatRoughnessMap,_.clearcoatRoughnessMapTransform)),m.clearcoatNormalMap&&(_.clearcoatNormalMap.value=m.clearcoatNormalMap,e(m.clearcoatNormalMap,_.clearcoatNormalMapTransform),_.clearcoatNormalScale.value.copy(m.clearcoatNormalScale),m.side===vn&&_.clearcoatNormalScale.value.negate())),m.iridescence>0&&(_.iridescence.value=m.iridescence,_.iridescenceIOR.value=m.iridescenceIOR,_.iridescenceThicknessMinimum.value=m.iridescenceThicknessRange[0],_.iridescenceThicknessMaximum.value=m.iridescenceThicknessRange[1],m.iridescenceMap&&(_.iridescenceMap.value=m.iridescenceMap,e(m.iridescenceMap,_.iridescenceMapTransform)),m.iridescenceThicknessMap&&(_.iridescenceThicknessMap.value=m.iridescenceThicknessMap,e(m.iridescenceThicknessMap,_.iridescenceThicknessMapTransform))),m.transmission>0&&(_.transmission.value=m.transmission,_.transmissionSamplerMap.value=C.texture,_.transmissionSamplerSize.value.set(C.width,C.height),m.transmissionMap&&(_.transmissionMap.value=m.transmissionMap,e(m.transmissionMap,_.transmissionMapTransform)),_.thickness.value=m.thickness,m.thicknessMap&&(_.thicknessMap.value=m.thicknessMap,e(m.thicknessMap,_.thicknessMapTransform)),_.attenuationDistance.value=m.attenuationDistance,_.attenuationColor.value.copy(m.attenuationColor)),m.anisotropy>0&&(_.anisotropyVector.value.set(m.anisotropy*Math.cos(m.anisotropyRotation),m.anisotropy*Math.sin(m.anisotropyRotation)),m.anisotropyMap&&(_.anisotropyMap.value=m.anisotropyMap,e(m.anisotropyMap,_.anisotropyMapTransform))),_.specularIntensity.value=m.specularIntensity,_.specularColor.value.copy(m.specularColor),m.specularColorMap&&(_.specularColorMap.value=m.specularColorMap,e(m.specularColorMap,_.specularColorMapTransform)),m.specularIntensityMap&&(_.specularIntensityMap.value=m.specularIntensityMap,e(m.specularIntensityMap,_.specularIntensityMapTransform))}function y(_,m){m.matcap&&(_.matcap.value=m.matcap)}function E(_,m){const C=t.get(m).light;_.referencePosition.value.setFromMatrixPosition(C.matrixWorld),_.nearDistance.value=C.shadow.camera.near,_.farDistance.value=C.shadow.camera.far}return{refreshFogUniforms:n,refreshMaterialUniforms:r}}function Rx(i,t,e,n){let r={},s={},o=[];const a=e.isWebGL2?i.getParameter(i.MAX_UNIFORM_BUFFER_BINDINGS):0;function c(C,w){const b=w.program;n.uniformBlockBinding(C,b)}function h(C,w){let b=r[C.id];b===void 0&&(y(C),b=u(C),r[C.id]=b,C.addEventListener("dispose",_));const N=w.program;n.updateUBOMapping(C,N);const U=t.render.frame;s[C.id]!==U&&(f(C),s[C.id]=U)}function u(C){const w=d();C.__bindingPointIndex=w;const b=i.createBuffer(),N=C.__size,U=C.usage;return i.bindBuffer(i.UNIFORM_BUFFER,b),i.bufferData(i.UNIFORM_BUFFER,N,U),i.bindBuffer(i.UNIFORM_BUFFER,null),i.bindBufferBase(i.UNIFORM_BUFFER,w,b),b}function d(){for(let C=0;C<a;C++)if(o.indexOf(C)===-1)return o.push(C),C;return console.error("THREE.WebGLRenderer: Maximum number of simultaneously usable uniforms groups reached."),0}function f(C){const w=r[C.id],b=C.uniforms,N=C.__cache;i.bindBuffer(i.UNIFORM_BUFFER,w);for(let U=0,T=b.length;U<T;U++){const M=Array.isArray(b[U])?b[U]:[b[U]];for(let v=0,S=M.length;v<S;v++){const P=M[v];if(p(P,U,v,N)===!0){const L=P.__offset,R=Array.isArray(P.value)?P.value:[P.value];let V=0;for(let G=0;G<R.length;G++){const K=R[G],Z=E(K);typeof K=="number"||typeof K=="boolean"?(P.__data[0]=K,i.bufferSubData(i.UNIFORM_BUFFER,L+V,P.__data)):K.isMatrix3?(P.__data[0]=K.elements[0],P.__data[1]=K.elements[1],P.__data[2]=K.elements[2],P.__data[3]=0,P.__data[4]=K.elements[3],P.__data[5]=K.elements[4],P.__data[6]=K.elements[5],P.__data[7]=0,P.__data[8]=K.elements[6],P.__data[9]=K.elements[7],P.__data[10]=K.elements[8],P.__data[11]=0):(K.toArray(P.__data,V),V+=Z.storage/Float32Array.BYTES_PER_ELEMENT)}i.bufferSubData(i.UNIFORM_BUFFER,L,P.__data)}}}i.bindBuffer(i.UNIFORM_BUFFER,null)}function p(C,w,b,N){const U=C.value,T=w+"_"+b;if(N[T]===void 0)return typeof U=="number"||typeof U=="boolean"?N[T]=U:N[T]=U.clone(),!0;{const M=N[T];if(typeof U=="number"||typeof U=="boolean"){if(M!==U)return N[T]=U,!0}else if(M.equals(U)===!1)return M.copy(U),!0}return!1}function y(C){const w=C.uniforms;let b=0;const N=16;for(let T=0,M=w.length;T<M;T++){const v=Array.isArray(w[T])?w[T]:[w[T]];for(let S=0,P=v.length;S<P;S++){const L=v[S],R=Array.isArray(L.value)?L.value:[L.value];for(let V=0,G=R.length;V<G;V++){const K=R[V],Z=E(K),J=b%N;J!==0&&N-J<Z.boundary&&(b+=N-J),L.__data=new Float32Array(Z.storage/Float32Array.BYTES_PER_ELEMENT),L.__offset=b,b+=Z.storage}}}const U=b%N;return U>0&&(b+=N-U),C.__size=b,C.__cache={},this}function E(C){const w={boundary:0,storage:0};return typeof C=="number"||typeof C=="boolean"?(w.boundary=4,w.storage=4):C.isVector2?(w.boundary=8,w.storage=8):C.isVector3||C.isColor?(w.boundary=16,w.storage=12):C.isVector4?(w.boundary=16,w.storage=16):C.isMatrix3?(w.boundary=48,w.storage=48):C.isMatrix4?(w.boundary=64,w.storage=64):C.isTexture?console.warn("THREE.WebGLRenderer: Texture samplers can not be part of an uniforms group."):console.warn("THREE.WebGLRenderer: Unsupported uniform value type.",C),w}function _(C){const w=C.target;w.removeEventListener("dispose",_);const b=o.indexOf(w.__bindingPointIndex);o.splice(b,1),i.deleteBuffer(r[w.id]),delete r[w.id],delete s[w.id]}function m(){for(const C in r)i.deleteBuffer(r[C]);o=[],r={},s={}}return{bind:c,update:h,dispose:m}}class Sp{constructor(t={}){const{canvas:e=M_(),context:n=null,depth:r=!0,stencil:s=!0,alpha:o=!1,antialias:a=!1,premultipliedAlpha:c=!0,preserveDrawingBuffer:h=!1,powerPreference:u="default",failIfMajorPerformanceCaveat:d=!1}=t;this.isWebGLRenderer=!0;let f;n!==null?f=n.getContextAttributes().alpha:f=o;const p=new Uint32Array(4),y=new Int32Array(4);let E=null,_=null;const m=[],C=[];this.domElement=e,this.debug={checkShaderErrors:!0,onShaderError:null},this.autoClear=!0,this.autoClearColor=!0,this.autoClearDepth=!0,this.autoClearStencil=!0,this.sortObjects=!0,this.clippingPlanes=[],this.localClippingEnabled=!1,this._outputColorSpace=Ze,this._useLegacyLights=!1,this.toneMapping=Li,this.toneMappingExposure=1;const w=this;let b=!1,N=0,U=0,T=null,M=-1,v=null;const S=new Xe,P=new Xe;let L=null;const R=new oe(0);let V=0,G=e.width,K=e.height,Z=1,J=null,et=null;const tt=new Xe(0,0,G,K),pt=new Xe(0,0,G,K);let ht=!1;const X=new zl;let it=!1,mt=!1,Pt=null;const Mt=new Be,Gt=new Vt,Ft=new k,wt={background:null,fog:null,environment:null,overrideMaterial:null,isScene:!0};function Wt(){return T===null?Z:1}let W=n;function pe(D,H){for(let Y=0;Y<D.length;Y++){const $=D[Y],j=e.getContext($,H);if(j!==null)return j}return null}try{const D={alpha:!0,depth:r,stencil:s,antialias:a,premultipliedAlpha:c,preserveDrawingBuffer:h,powerPreference:u,failIfMajorPerformanceCaveat:d};if("setAttribute"in e&&e.setAttribute("data-engine",`three.js r${Ol}`),e.addEventListener("webglcontextlost",at,!1),e.addEventListener("webglcontextrestored",B,!1),e.addEventListener("webglcontextcreationerror",ft,!1),W===null){const H=["webgl2","webgl","experimental-webgl"];if(w.isWebGL1Renderer===!0&&H.shift(),W=pe(H,D),W===null)throw pe(H)?new Error("Error creating WebGL context with your selected attributes."):new Error("Error creating WebGL context.")}typeof WebGLRenderingContext<"u"&&W instanceof WebGLRenderingContext&&console.warn("THREE.WebGLRenderer: WebGL 1 support was deprecated in r153 and will be removed in r163."),W.getShaderPrecisionFormat===void 0&&(W.getShaderPrecisionFormat=function(){return{rangeMin:1,rangeMax:1,precision:1}})}catch(D){throw console.error("THREE.WebGLRenderer: "+D.message),D}let Rt,kt,bt,ye,te,O,A,q,lt,rt,ct,Ct,ut,_t,Nt,Qt,st,he,Kt,qt,Lt,Et,F,dt;function It(){Rt=new Vy(W),kt=new Dy(W,Rt,t),Rt.init(kt),Et=new Tx(W,Rt,kt),bt=new xx(W,Rt,kt),ye=new Hy(W),te=new ax,O=new Sx(W,Rt,bt,te,kt,Et,ye),A=new Ny(w),q=new Fy(w),lt=new $_(W,kt),F=new Iy(W,Rt,lt,kt),rt=new By(W,lt,ye,F),ct=new qy(W,rt,lt,ye),Kt=new Wy(W,kt,O),Qt=new Ly(te),Ct=new ox(w,A,q,Rt,kt,F,Qt),ut=new Ax(w,te),_t=new lx,Nt=new mx(Rt,kt),he=new Cy(w,A,q,bt,ct,f,c),st=new Ex(w,ct,kt),dt=new Rx(W,ye,kt,bt),qt=new Py(W,Rt,ye,kt),Lt=new ky(W,Rt,ye,kt),ye.programs=Ct.programs,w.capabilities=kt,w.extensions=Rt,w.properties=te,w.renderLists=_t,w.shadowMap=st,w.state=bt,w.info=ye}It();const St=new bx(w,W);this.xr=St,this.getContext=function(){return W},this.getContextAttributes=function(){return W.getContextAttributes()},this.forceContextLoss=function(){const D=Rt.get("WEBGL_lose_context");D&&D.loseContext()},this.forceContextRestore=function(){const D=Rt.get("WEBGL_lose_context");D&&D.restoreContext()},this.getPixelRatio=function(){return Z},this.setPixelRatio=function(D){D!==void 0&&(Z=D,this.setSize(G,K,!1))},this.getSize=function(D){return D.set(G,K)},this.setSize=function(D,H,Y=!0){if(St.isPresenting){console.warn("THREE.WebGLRenderer: Can't change size while VR device is presenting.");return}G=D,K=H,e.width=Math.floor(D*Z),e.height=Math.floor(H*Z),Y===!0&&(e.style.width=D+"px",e.style.height=H+"px"),this.setViewport(0,0,D,H)},this.getDrawingBufferSize=function(D){return D.set(G*Z,K*Z).floor()},this.setDrawingBufferSize=function(D,H,Y){G=D,K=H,Z=Y,e.width=Math.floor(D*Y),e.height=Math.floor(H*Y),this.setViewport(0,0,D,H)},this.getCurrentViewport=function(D){return D.copy(S)},this.getViewport=function(D){return D.copy(tt)},this.setViewport=function(D,H,Y,$){D.isVector4?tt.set(D.x,D.y,D.z,D.w):tt.set(D,H,Y,$),bt.viewport(S.copy(tt).multiplyScalar(Z).floor())},this.getScissor=function(D){return D.copy(pt)},this.setScissor=function(D,H,Y,$){D.isVector4?pt.set(D.x,D.y,D.z,D.w):pt.set(D,H,Y,$),bt.scissor(P.copy(pt).multiplyScalar(Z).floor())},this.getScissorTest=function(){return ht},this.setScissorTest=function(D){bt.setScissorTest(ht=D)},this.setOpaqueSort=function(D){J=D},this.setTransparentSort=function(D){et=D},this.getClearColor=function(D){return D.copy(he.getClearColor())},this.setClearColor=function(){he.setClearColor.apply(he,arguments)},this.getClearAlpha=function(){return he.getClearAlpha()},this.setClearAlpha=function(){he.setClearAlpha.apply(he,arguments)},this.clear=function(D=!0,H=!0,Y=!0){let $=0;if(D){let j=!1;if(T!==null){const xt=T.texture.format;j=xt===Jf||xt===Zf||xt===Qf}if(j){const xt=T.texture.type,Dt=xt===Ni||xt===Ri||xt===Fl||xt===or||xt===$f||xt===Kf,Ht=he.getClearColor(),zt=he.getClearAlpha(),ee=Ht.r,Yt=Ht.g,jt=Ht.b;Dt?(p[0]=ee,p[1]=Yt,p[2]=jt,p[3]=zt,W.clearBufferuiv(W.COLOR,0,p)):(y[0]=ee,y[1]=Yt,y[2]=jt,y[3]=zt,W.clearBufferiv(W.COLOR,0,y))}else $|=W.COLOR_BUFFER_BIT}H&&($|=W.DEPTH_BUFFER_BIT),Y&&($|=W.STENCIL_BUFFER_BIT,this.state.buffers.stencil.setMask(4294967295)),W.clear($)},this.clearColor=function(){this.clear(!0,!1,!1)},this.clearDepth=function(){this.clear(!1,!0,!1)},this.clearStencil=function(){this.clear(!1,!1,!0)},this.dispose=function(){e.removeEventListener("webglcontextlost",at,!1),e.removeEventListener("webglcontextrestored",B,!1),e.removeEventListener("webglcontextcreationerror",ft,!1),_t.dispose(),Nt.dispose(),te.dispose(),A.dispose(),q.dispose(),ct.dispose(),F.dispose(),dt.dispose(),Ct.dispose(),St.dispose(),St.removeEventListener("sessionstart",Ae),St.removeEventListener("sessionend",Xt),Pt&&(Pt.dispose(),Pt=null),Ce.stop()};function at(D){D.preventDefault(),console.log("THREE.WebGLRenderer: Context Lost."),b=!0}function B(){console.log("THREE.WebGLRenderer: Context Restored."),b=!1;const D=ye.autoReset,H=st.enabled,Y=st.autoUpdate,$=st.needsUpdate,j=st.type;It(),ye.autoReset=D,st.enabled=H,st.autoUpdate=Y,st.needsUpdate=$,st.type=j}function ft(D){console.error("THREE.WebGLRenderer: A WebGL context could not be created. Reason: ",D.statusMessage)}function yt(D){const H=D.target;H.removeEventListener("dispose",yt),Ut(H)}function Ut(D){Ot(D),te.remove(D)}function Ot(D){const H=te.get(D).programs;H!==void 0&&(H.forEach(function(Y){Ct.releaseProgram(Y)}),D.isShaderMaterial&&Ct.releaseShaderCache(D))}this.renderBufferDirect=function(D,H,Y,$,j,xt){H===null&&(H=wt);const Dt=j.isMesh&&j.matrixWorld.determinant()<0,Ht=To(D,H,Y,$,j);bt.setMaterial($,Dt);let zt=Y.index,ee=1;if($.wireframe===!0){if(zt=rt.getWireframeAttribute(Y),zt===void 0)return;ee=2}const Yt=Y.drawRange,jt=Y.attributes.position;let me=Yt.start*ee,$e=(Yt.start+Yt.count)*ee;xt!==null&&(me=Math.max(me,xt.start*ee),$e=Math.min($e,(xt.start+xt.count)*ee)),zt!==null?(me=Math.max(me,0),$e=Math.min($e,zt.count)):jt!=null&&(me=Math.max(me,0),$e=Math.min($e,jt.count));const Ne=$e-me;if(Ne<0||Ne===1/0)return;F.setup(j,$,Ht,Y,zt);let An,Se=qt;if(zt!==null&&(An=lt.get(zt),Se=Lt,Se.setIndex(An)),j.isMesh)$.wireframe===!0?(bt.setLineWidth($.wireframeLinewidth*Wt()),Se.setMode(W.LINES)):Se.setMode(W.TRIANGLES);else if(j.isLine){let ie=$.linewidth;ie===void 0&&(ie=1),bt.setLineWidth(ie*Wt()),j.isLineSegments?Se.setMode(W.LINES):j.isLineLoop?Se.setMode(W.LINE_LOOP):Se.setMode(W.LINE_STRIP)}else j.isPoints?Se.setMode(W.POINTS):j.isSprite&&Se.setMode(W.TRIANGLES);if(j.isBatchedMesh)Se.renderMultiDraw(j._multiDrawStarts,j._multiDrawCounts,j._multiDrawCount);else if(j.isInstancedMesh)Se.renderInstances(me,Ne,j.count);else if(Y.isInstancedBufferGeometry){const ie=Y._maxInstanceCount!==void 0?Y._maxInstanceCount:1/0,qi=Math.min(Y.instanceCount,ie);Se.renderInstances(me,Ne,qi)}else Se.render(me,Ne)};function ne(D,H,Y){D.transparent===!0&&D.side===_n&&D.forceSinglePass===!1?(D.side=vn,D.needsUpdate=!0,Er(D,H,Y),D.side=ui,D.needsUpdate=!0,Er(D,H,Y),D.side=_n):Er(D,H,Y)}this.compile=function(D,H,Y=null){Y===null&&(Y=D),_=Nt.get(Y),_.init(),C.push(_),Y.traverseVisible(function(j){j.isLight&&j.layers.test(H.layers)&&(_.pushLight(j),j.castShadow&&_.pushShadow(j))}),D!==Y&&D.traverseVisible(function(j){j.isLight&&j.layers.test(H.layers)&&(_.pushLight(j),j.castShadow&&_.pushShadow(j))}),_.setupLights(w._useLegacyLights);const $=new Set;return D.traverse(function(j){const xt=j.material;if(xt)if(Array.isArray(xt))for(let Dt=0;Dt<xt.length;Dt++){const Ht=xt[Dt];ne(Ht,Y,j),$.add(Ht)}else ne(xt,Y,j),$.add(xt)}),C.pop(),_=null,$},this.compileAsync=function(D,H,Y=null){const $=this.compile(D,H,Y);return new Promise(j=>{function xt(){if($.forEach(function(Dt){te.get(Dt).currentProgram.isReady()&&$.delete(Dt)}),$.size===0){j(D);return}setTimeout(xt,10)}Rt.get("KHR_parallel_shader_compile")!==null?xt():setTimeout(xt,10)})};let se=null;function _e(D){se&&se(D)}function Ae(){Ce.stop()}function Xt(){Ce.start()}const Ce=new pp;Ce.setAnimationLoop(_e),typeof self<"u"&&Ce.setContext(self),this.setAnimationLoop=function(D){se=D,St.setAnimationLoop(D),D===null?Ce.stop():Ce.start()},St.addEventListener("sessionstart",Ae),St.addEventListener("sessionend",Xt),this.render=function(D,H){if(H!==void 0&&H.isCamera!==!0){console.error("THREE.WebGLRenderer.render: camera is not an instance of THREE.Camera.");return}if(b===!0)return;D.matrixWorldAutoUpdate===!0&&D.updateMatrixWorld(),H.parent===null&&H.matrixWorldAutoUpdate===!0&&H.updateMatrixWorld(),St.enabled===!0&&St.isPresenting===!0&&(St.cameraAutoUpdate===!0&&St.updateCamera(H),H=St.getCamera()),D.isScene===!0&&D.onBeforeRender(w,D,H,T),_=Nt.get(D,C.length),_.init(),C.push(_),Mt.multiplyMatrices(H.projectionMatrix,H.matrixWorldInverse),X.setFromProjectionMatrix(Mt),mt=this.localClippingEnabled,it=Qt.init(this.clippingPlanes,mt),E=_t.get(D,m.length),E.init(),m.push(E),nn(D,H,0,w.sortObjects),E.finish(),w.sortObjects===!0&&E.sort(J,et),this.info.render.frame++,it===!0&&Qt.beginShadows();const Y=_.state.shadowsArray;if(st.render(Y,D,H),it===!0&&Qt.endShadows(),this.info.autoReset===!0&&this.info.reset(),he.render(E,D),_.setupLights(w._useLegacyLights),H.isArrayCamera){const $=H.cameras;for(let j=0,xt=$.length;j<xt;j++){const Dt=$[j];_i(E,D,Dt,Dt.viewport)}}else _i(E,D,H);T!==null&&(O.updateMultisampleRenderTarget(T),O.updateRenderTargetMipmap(T)),D.isScene===!0&&D.onAfterRender(w,D,H),F.resetDefaultState(),M=-1,v=null,C.pop(),C.length>0?_=C[C.length-1]:_=null,m.pop(),m.length>0?E=m[m.length-1]:E=null};function nn(D,H,Y,$){if(D.visible===!1)return;if(D.layers.test(H.layers)){if(D.isGroup)Y=D.renderOrder;else if(D.isLOD)D.autoUpdate===!0&&D.update(H);else if(D.isLight)_.pushLight(D),D.castShadow&&_.pushShadow(D);else if(D.isSprite){if(!D.frustumCulled||X.intersectsSprite(D)){$&&Ft.setFromMatrixPosition(D.matrixWorld).applyMatrix4(Mt);const Dt=ct.update(D),Ht=D.material;Ht.visible&&E.push(D,Dt,Ht,Y,Ft.z,null)}}else if((D.isMesh||D.isLine||D.isPoints)&&(!D.frustumCulled||X.intersectsObject(D))){const Dt=ct.update(D),Ht=D.material;if($&&(D.boundingSphere!==void 0?(D.boundingSphere===null&&D.computeBoundingSphere(),Ft.copy(D.boundingSphere.center)):(Dt.boundingSphere===null&&Dt.computeBoundingSphere(),Ft.copy(Dt.boundingSphere.center)),Ft.applyMatrix4(D.matrixWorld).applyMatrix4(Mt)),Array.isArray(Ht)){const zt=Dt.groups;for(let ee=0,Yt=zt.length;ee<Yt;ee++){const jt=zt[ee],me=Ht[jt.materialIndex];me&&me.visible&&E.push(D,Dt,me,Y,Ft.z,jt)}}else Ht.visible&&E.push(D,Dt,Ht,Y,Ft.z,null)}}const xt=D.children;for(let Dt=0,Ht=xt.length;Dt<Ht;Dt++)nn(xt[Dt],H,Y,$)}function _i(D,H,Y,$){const j=D.opaque,xt=D.transmissive,Dt=D.transparent;_.setupLightsView(Y),it===!0&&Qt.setGlobalState(w.clippingPlanes,Y),xt.length>0&&oc(j,xt,H,Y),$&&bt.viewport(S.copy($)),j.length>0&&yr(j,H,Y),xt.length>0&&yr(xt,H,Y),Dt.length>0&&yr(Dt,H,Y),bt.buffers.depth.setTest(!0),bt.buffers.depth.setMask(!0),bt.buffers.color.setMask(!0),bt.setPolygonOffset(!1)}function oc(D,H,Y,$){if((Y.isScene===!0?Y.overrideMaterial:null)!==null)return;const xt=kt.isWebGL2;Pt===null&&(Pt=new hr(1,1,{generateMipmaps:!0,type:Rt.has("EXT_color_buffer_half_float")?eo:Ni,minFilter:to,samples:xt?4:0})),w.getDrawingBufferSize(Gt),xt?Pt.setSize(Gt.x,Gt.y):Pt.setSize(fl(Gt.x),fl(Gt.y));const Dt=w.getRenderTarget();w.setRenderTarget(Pt),w.getClearColor(R),V=w.getClearAlpha(),V<1&&w.setClearColor(16777215,.5),w.clear();const Ht=w.toneMapping;w.toneMapping=Li,yr(D,Y,$),O.updateMultisampleRenderTarget(Pt),O.updateRenderTargetMipmap(Pt);let zt=!1;for(let ee=0,Yt=H.length;ee<Yt;ee++){const jt=H[ee],me=jt.object,$e=jt.geometry,Ne=jt.material,An=jt.group;if(Ne.side===_n&&me.layers.test($.layers)){const Se=Ne.side;Ne.side=vn,Ne.needsUpdate=!0,Jn(me,Y,$,$e,Ne,An),Ne.side=Se,Ne.needsUpdate=!0,zt=!0}}zt===!0&&(O.updateMultisampleRenderTarget(Pt),O.updateRenderTargetMipmap(Pt)),w.setRenderTarget(Dt),w.setClearColor(R,V),w.toneMapping=Ht}function yr(D,H,Y){const $=H.isScene===!0?H.overrideMaterial:null;for(let j=0,xt=D.length;j<xt;j++){const Dt=D[j],Ht=Dt.object,zt=Dt.geometry,ee=$===null?Dt.material:$,Yt=Dt.group;Ht.layers.test(Y.layers)&&Jn(Ht,H,Y,zt,ee,Yt)}}function Jn(D,H,Y,$,j,xt){D.onBeforeRender(w,H,Y,$,j,xt),D.modelViewMatrix.multiplyMatrices(Y.matrixWorldInverse,D.matrixWorld),D.normalMatrix.getNormalMatrix(D.modelViewMatrix),j.onBeforeRender(w,H,Y,$,D,xt),j.transparent===!0&&j.side===_n&&j.forceSinglePass===!1?(j.side=vn,j.needsUpdate=!0,w.renderBufferDirect(Y,H,$,j,D,xt),j.side=ui,j.needsUpdate=!0,w.renderBufferDirect(Y,H,$,j,D,xt),j.side=_n):w.renderBufferDirect(Y,H,$,j,D,xt),D.onAfterRender(w,H,Y,$,j,xt)}function Er(D,H,Y){H.isScene!==!0&&(H=wt);const $=te.get(D),j=_.state.lights,xt=_.state.shadowsArray,Dt=j.state.version,Ht=Ct.getParameters(D,j.state,xt,H,Y),zt=Ct.getProgramCacheKey(Ht);let ee=$.programs;$.environment=D.isMeshStandardMaterial?H.environment:null,$.fog=H.fog,$.envMap=(D.isMeshStandardMaterial?q:A).get(D.envMap||$.environment),ee===void 0&&(D.addEventListener("dispose",yt),ee=new Map,$.programs=ee);let Yt=ee.get(zt);if(Yt!==void 0){if($.currentProgram===Yt&&$.lightsStateVersion===Dt)return Wi(D,Ht),Yt}else Ht.uniforms=Ct.getUniforms(D),D.onBuild(Y,Ht,w),D.onBeforeCompile(Ht,w),Yt=Ct.acquireProgram(Ht,zt),ee.set(zt,Yt),$.uniforms=Ht.uniforms;const jt=$.uniforms;return(!D.isShaderMaterial&&!D.isRawShaderMaterial||D.clipping===!0)&&(jt.clippingPlanes=Qt.uniform),Wi(D,Ht),$.needsLights=xr(D),$.lightsStateVersion=Dt,$.needsLights&&(jt.ambientLightColor.value=j.state.ambient,jt.lightProbe.value=j.state.probe,jt.directionalLights.value=j.state.directional,jt.directionalLightShadows.value=j.state.directionalShadow,jt.spotLights.value=j.state.spot,jt.spotLightShadows.value=j.state.spotShadow,jt.rectAreaLights.value=j.state.rectArea,jt.ltc_1.value=j.state.rectAreaLTC1,jt.ltc_2.value=j.state.rectAreaLTC2,jt.pointLights.value=j.state.point,jt.pointLightShadows.value=j.state.pointShadow,jt.hemisphereLights.value=j.state.hemi,jt.directionalShadowMap.value=j.state.directionalShadowMap,jt.directionalShadowMatrix.value=j.state.directionalShadowMatrix,jt.spotShadowMap.value=j.state.spotShadowMap,jt.spotLightMatrix.value=j.state.spotLightMatrix,jt.spotLightMap.value=j.state.spotLightMap,jt.pointShadowMap.value=j.state.pointShadowMap,jt.pointShadowMatrix.value=j.state.pointShadowMatrix),$.currentProgram=Yt,$.uniformsList=null,Yt}function So(D){if(D.uniformsList===null){const H=D.currentProgram.getUniforms();D.uniformsList=pa.seqWithValue(H.seq,D.uniforms)}return D.uniformsList}function Wi(D,H){const Y=te.get(D);Y.outputColorSpace=H.outputColorSpace,Y.batching=H.batching,Y.instancing=H.instancing,Y.instancingColor=H.instancingColor,Y.skinning=H.skinning,Y.morphTargets=H.morphTargets,Y.morphNormals=H.morphNormals,Y.morphColors=H.morphColors,Y.morphTargetsCount=H.morphTargetsCount,Y.numClippingPlanes=H.numClippingPlanes,Y.numIntersection=H.numClipIntersection,Y.vertexAlphas=H.vertexAlphas,Y.vertexTangents=H.vertexTangents,Y.toneMapping=H.toneMapping}function To(D,H,Y,$,j){H.isScene!==!0&&(H=wt),O.resetTextureUnits();const xt=H.fog,Dt=$.isMeshStandardMaterial?H.environment:null,Ht=T===null?w.outputColorSpace:T.isXRRenderTarget===!0?T.texture.colorSpace:di,zt=($.isMeshStandardMaterial?q:A).get($.envMap||Dt),ee=$.vertexColors===!0&&!!Y.attributes.color&&Y.attributes.color.itemSize===4,Yt=!!Y.attributes.tangent&&(!!$.normalMap||$.anisotropy>0),jt=!!Y.morphAttributes.position,me=!!Y.morphAttributes.normal,$e=!!Y.morphAttributes.color;let Ne=Li;$.toneMapped&&(T===null||T.isXRRenderTarget===!0)&&(Ne=w.toneMapping);const An=Y.morphAttributes.position||Y.morphAttributes.normal||Y.morphAttributes.color,Se=An!==void 0?An.length:0,ie=te.get($),qi=_.state.lights;if(it===!0&&(mt===!0||D!==v)){const Ue=D===v&&$.id===M;Qt.setState($,D,Ue)}let Te=!1;$.version===ie.__version?(ie.needsLights&&ie.lightsStateVersion!==qi.state.version||ie.outputColorSpace!==Ht||j.isBatchedMesh&&ie.batching===!1||!j.isBatchedMesh&&ie.batching===!0||j.isInstancedMesh&&ie.instancing===!1||!j.isInstancedMesh&&ie.instancing===!0||j.isSkinnedMesh&&ie.skinning===!1||!j.isSkinnedMesh&&ie.skinning===!0||j.isInstancedMesh&&ie.instancingColor===!0&&j.instanceColor===null||j.isInstancedMesh&&ie.instancingColor===!1&&j.instanceColor!==null||ie.envMap!==zt||$.fog===!0&&ie.fog!==xt||ie.numClippingPlanes!==void 0&&(ie.numClippingPlanes!==Qt.numPlanes||ie.numIntersection!==Qt.numIntersection)||ie.vertexAlphas!==ee||ie.vertexTangents!==Yt||ie.morphTargets!==jt||ie.morphNormals!==me||ie.morphColors!==$e||ie.toneMapping!==Ne||kt.isWebGL2===!0&&ie.morphTargetsCount!==Se)&&(Te=!0):(Te=!0,ie.__version=$.version);let jn=ie.currentProgram;Te===!0&&(jn=Er($,H,j));let bs=!1,Xi=!1,As=!1;const ke=jn.getUniforms(),Yn=ie.uniforms;if(bt.useProgram(jn.program)&&(bs=!0,Xi=!0,As=!0),$.id!==M&&(M=$.id,Xi=!0),bs||v!==D){ke.setValue(W,"projectionMatrix",D.projectionMatrix),ke.setValue(W,"viewMatrix",D.matrixWorldInverse);const Ue=ke.map.cameraPosition;Ue!==void 0&&Ue.setValue(W,Ft.setFromMatrixPosition(D.matrixWorld)),kt.logarithmicDepthBuffer&&ke.setValue(W,"logDepthBufFC",2/(Math.log(D.far+1)/Math.LN2)),($.isMeshPhongMaterial||$.isMeshToonMaterial||$.isMeshLambertMaterial||$.isMeshBasicMaterial||$.isMeshStandardMaterial||$.isShaderMaterial)&&ke.setValue(W,"isOrthographic",D.isOrthographicCamera===!0),v!==D&&(v=D,Xi=!0,As=!0)}if(j.isSkinnedMesh){ke.setOptional(W,j,"bindMatrix"),ke.setOptional(W,j,"bindMatrixInverse");const Ue=j.skeleton;Ue&&(kt.floatVertexTextures?(Ue.boneTexture===null&&Ue.computeBoneTexture(),ke.setValue(W,"boneTexture",Ue.boneTexture,O)):console.warn("THREE.WebGLRenderer: SkinnedMesh can only be used with WebGL 2. With WebGL 1 OES_texture_float and vertex textures support is required."))}j.isBatchedMesh&&(ke.setOptional(W,j,"batchingTexture"),ke.setValue(W,"batchingTexture",j._matricesTexture,O));const Rs=Y.morphAttributes;if((Rs.position!==void 0||Rs.normal!==void 0||Rs.color!==void 0&&kt.isWebGL2===!0)&&Kt.update(j,Y,jn),(Xi||ie.receiveShadow!==j.receiveShadow)&&(ie.receiveShadow=j.receiveShadow,ke.setValue(W,"receiveShadow",j.receiveShadow)),$.isMeshGouraudMaterial&&$.envMap!==null&&(Yn.envMap.value=zt,Yn.flipEnvMap.value=zt.isCubeTexture&&zt.isRenderTargetTexture===!1?-1:1),Xi&&(ke.setValue(W,"toneMappingExposure",w.toneMappingExposure),ie.needsLights&&ws(Yn,As),xt&&$.fog===!0&&ut.refreshFogUniforms(Yn,xt),ut.refreshMaterialUniforms(Yn,$,Z,K,Pt),pa.upload(W,So(ie),Yn,O)),$.isShaderMaterial&&$.uniformsNeedUpdate===!0&&(pa.upload(W,So(ie),Yn,O),$.uniformsNeedUpdate=!1),$.isSpriteMaterial&&ke.setValue(W,"center",j.center),ke.setValue(W,"modelViewMatrix",j.modelViewMatrix),ke.setValue(W,"normalMatrix",j.normalMatrix),ke.setValue(W,"modelMatrix",j.matrixWorld),$.isShaderMaterial||$.isRawShaderMaterial){const Ue=$.uniformsGroups;for(let Sn=0,Sr=Ue.length;Sn<Sr;Sn++)if(kt.isWebGL2){const ji=Ue[Sn];dt.update(ji,jn),dt.bind(ji,jn)}else console.warn("THREE.WebGLRenderer: Uniform Buffer Objects can only be used with WebGL 2.")}return jn}function ws(D,H){D.ambientLightColor.needsUpdate=H,D.lightProbe.needsUpdate=H,D.directionalLights.needsUpdate=H,D.directionalLightShadows.needsUpdate=H,D.pointLights.needsUpdate=H,D.pointLightShadows.needsUpdate=H,D.spotLights.needsUpdate=H,D.spotLightShadows.needsUpdate=H,D.rectAreaLights.needsUpdate=H,D.hemisphereLights.needsUpdate=H}function xr(D){return D.isMeshLambertMaterial||D.isMeshToonMaterial||D.isMeshPhongMaterial||D.isMeshStandardMaterial||D.isShadowMaterial||D.isShaderMaterial&&D.lights===!0}this.getActiveCubeFace=function(){return N},this.getActiveMipmapLevel=function(){return U},this.getRenderTarget=function(){return T},this.setRenderTargetTextures=function(D,H,Y){te.get(D.texture).__webglTexture=H,te.get(D.depthTexture).__webglTexture=Y;const $=te.get(D);$.__hasExternalTextures=!0,$.__hasExternalTextures&&($.__autoAllocateDepthBuffer=Y===void 0,$.__autoAllocateDepthBuffer||Rt.has("WEBGL_multisampled_render_to_texture")===!0&&(console.warn("THREE.WebGLRenderer: Render-to-texture extension was disabled because an external texture was provided"),$.__useRenderToTexture=!1))},this.setRenderTargetFramebuffer=function(D,H){const Y=te.get(D);Y.__webglFramebuffer=H,Y.__useDefaultFramebuffer=H===void 0},this.setRenderTarget=function(D,H=0,Y=0){T=D,N=H,U=Y;let $=!0,j=null,xt=!1,Dt=!1;if(D){const zt=te.get(D);zt.__useDefaultFramebuffer!==void 0?(bt.bindFramebuffer(W.FRAMEBUFFER,null),$=!1):zt.__webglFramebuffer===void 0?O.setupRenderTarget(D):zt.__hasExternalTextures&&O.rebindTextures(D,te.get(D.texture).__webglTexture,te.get(D.depthTexture).__webglTexture);const ee=D.texture;(ee.isData3DTexture||ee.isDataArrayTexture||ee.isCompressedArrayTexture)&&(Dt=!0);const Yt=te.get(D).__webglFramebuffer;D.isWebGLCubeRenderTarget?(Array.isArray(Yt[H])?j=Yt[H][Y]:j=Yt[H],xt=!0):kt.isWebGL2&&D.samples>0&&O.useMultisampledRTT(D)===!1?j=te.get(D).__webglMultisampledFramebuffer:Array.isArray(Yt)?j=Yt[Y]:j=Yt,S.copy(D.viewport),P.copy(D.scissor),L=D.scissorTest}else S.copy(tt).multiplyScalar(Z).floor(),P.copy(pt).multiplyScalar(Z).floor(),L=ht;if(bt.bindFramebuffer(W.FRAMEBUFFER,j)&&kt.drawBuffers&&$&&bt.drawBuffers(D,j),bt.viewport(S),bt.scissor(P),bt.setScissorTest(L),xt){const zt=te.get(D.texture);W.framebufferTexture2D(W.FRAMEBUFFER,W.COLOR_ATTACHMENT0,W.TEXTURE_CUBE_MAP_POSITIVE_X+H,zt.__webglTexture,Y)}else if(Dt){const zt=te.get(D.texture),ee=H||0;W.framebufferTextureLayer(W.FRAMEBUFFER,W.COLOR_ATTACHMENT0,zt.__webglTexture,Y||0,ee)}M=-1},this.readRenderTargetPixels=function(D,H,Y,$,j,xt,Dt){if(!(D&&D.isWebGLRenderTarget)){console.error("THREE.WebGLRenderer.readRenderTargetPixels: renderTarget is not THREE.WebGLRenderTarget.");return}let Ht=te.get(D).__webglFramebuffer;if(D.isWebGLCubeRenderTarget&&Dt!==void 0&&(Ht=Ht[Dt]),Ht){bt.bindFramebuffer(W.FRAMEBUFFER,Ht);try{const zt=D.texture,ee=zt.format,Yt=zt.type;if(ee!==Wn&&Et.convert(ee)!==W.getParameter(W.IMPLEMENTATION_COLOR_READ_FORMAT)){console.error("THREE.WebGLRenderer.readRenderTargetPixels: renderTarget is not in RGBA or implementation defined format.");return}const jt=Yt===eo&&(Rt.has("EXT_color_buffer_half_float")||kt.isWebGL2&&Rt.has("EXT_color_buffer_float"));if(Yt!==Ni&&Et.convert(Yt)!==W.getParameter(W.IMPLEMENTATION_COLOR_READ_TYPE)&&!(Yt===Ci&&(kt.isWebGL2||Rt.has("OES_texture_float")||Rt.has("WEBGL_color_buffer_float")))&&!jt){console.error("THREE.WebGLRenderer.readRenderTargetPixels: renderTarget is not in UnsignedByteType or implementation defined type.");return}H>=0&&H<=D.width-$&&Y>=0&&Y<=D.height-j&&W.readPixels(H,Y,$,j,Et.convert(ee),Et.convert(Yt),xt)}finally{const zt=T!==null?te.get(T).__webglFramebuffer:null;bt.bindFramebuffer(W.FRAMEBUFFER,zt)}}},this.copyFramebufferToTexture=function(D,H,Y=0){const $=Math.pow(2,-Y),j=Math.floor(H.image.width*$),xt=Math.floor(H.image.height*$);O.setTexture2D(H,0),W.copyTexSubImage2D(W.TEXTURE_2D,Y,0,0,D.x,D.y,j,xt),bt.unbindTexture()},this.copyTextureToTexture=function(D,H,Y,$=0){const j=H.image.width,xt=H.image.height,Dt=Et.convert(Y.format),Ht=Et.convert(Y.type);O.setTexture2D(Y,0),W.pixelStorei(W.UNPACK_FLIP_Y_WEBGL,Y.flipY),W.pixelStorei(W.UNPACK_PREMULTIPLY_ALPHA_WEBGL,Y.premultiplyAlpha),W.pixelStorei(W.UNPACK_ALIGNMENT,Y.unpackAlignment),H.isDataTexture?W.texSubImage2D(W.TEXTURE_2D,$,D.x,D.y,j,xt,Dt,Ht,H.image.data):H.isCompressedTexture?W.compressedTexSubImage2D(W.TEXTURE_2D,$,D.x,D.y,H.mipmaps[0].width,H.mipmaps[0].height,Dt,H.mipmaps[0].data):W.texSubImage2D(W.TEXTURE_2D,$,D.x,D.y,Dt,Ht,H.image),$===0&&Y.generateMipmaps&&W.generateMipmap(W.TEXTURE_2D),bt.unbindTexture()},this.copyTextureToTexture3D=function(D,H,Y,$,j=0){if(w.isWebGL1Renderer){console.warn("THREE.WebGLRenderer.copyTextureToTexture3D: can only be used with WebGL2.");return}const xt=D.max.x-D.min.x+1,Dt=D.max.y-D.min.y+1,Ht=D.max.z-D.min.z+1,zt=Et.convert($.format),ee=Et.convert($.type);let Yt;if($.isData3DTexture)O.setTexture3D($,0),Yt=W.TEXTURE_3D;else if($.isDataArrayTexture||$.isCompressedArrayTexture)O.setTexture2DArray($,0),Yt=W.TEXTURE_2D_ARRAY;else{console.warn("THREE.WebGLRenderer.copyTextureToTexture3D: only supports THREE.DataTexture3D and THREE.DataTexture2DArray.");return}W.pixelStorei(W.UNPACK_FLIP_Y_WEBGL,$.flipY),W.pixelStorei(W.UNPACK_PREMULTIPLY_ALPHA_WEBGL,$.premultiplyAlpha),W.pixelStorei(W.UNPACK_ALIGNMENT,$.unpackAlignment);const jt=W.getParameter(W.UNPACK_ROW_LENGTH),me=W.getParameter(W.UNPACK_IMAGE_HEIGHT),$e=W.getParameter(W.UNPACK_SKIP_PIXELS),Ne=W.getParameter(W.UNPACK_SKIP_ROWS),An=W.getParameter(W.UNPACK_SKIP_IMAGES),Se=Y.isCompressedTexture?Y.mipmaps[j]:Y.image;W.pixelStorei(W.UNPACK_ROW_LENGTH,Se.width),W.pixelStorei(W.UNPACK_IMAGE_HEIGHT,Se.height),W.pixelStorei(W.UNPACK_SKIP_PIXELS,D.min.x),W.pixelStorei(W.UNPACK_SKIP_ROWS,D.min.y),W.pixelStorei(W.UNPACK_SKIP_IMAGES,D.min.z),Y.isDataTexture||Y.isData3DTexture?W.texSubImage3D(Yt,j,H.x,H.y,H.z,xt,Dt,Ht,zt,ee,Se.data):Y.isCompressedArrayTexture?(console.warn("THREE.WebGLRenderer.copyTextureToTexture3D: untested support for compressed srcTexture."),W.compressedTexSubImage3D(Yt,j,H.x,H.y,H.z,xt,Dt,Ht,zt,Se.data)):W.texSubImage3D(Yt,j,H.x,H.y,H.z,xt,Dt,Ht,zt,ee,Se),W.pixelStorei(W.UNPACK_ROW_LENGTH,jt),W.pixelStorei(W.UNPACK_IMAGE_HEIGHT,me),W.pixelStorei(W.UNPACK_SKIP_PIXELS,$e),W.pixelStorei(W.UNPACK_SKIP_ROWS,Ne),W.pixelStorei(W.UNPACK_SKIP_IMAGES,An),j===0&&$.generateMipmaps&&W.generateMipmap(Yt),bt.unbindTexture()},this.initTexture=function(D){D.isCubeTexture?O.setTextureCube(D,0):D.isData3DTexture?O.setTexture3D(D,0):D.isDataArrayTexture||D.isCompressedArrayTexture?O.setTexture2DArray(D,0):O.setTexture2D(D,0),bt.unbindTexture()},this.resetState=function(){N=0,U=0,T=null,bt.reset(),F.reset()},typeof __THREE_DEVTOOLS__<"u"&&__THREE_DEVTOOLS__.dispatchEvent(new CustomEvent("observe",{detail:this}))}get coordinateSystem(){return li}get outputColorSpace(){return this._outputColorSpace}set outputColorSpace(t){this._outputColorSpace=t;const e=this.getContext();e.drawingBufferColorSpace=t===Vl?"display-p3":"srgb",e.unpackColorSpace=ve.workingColorSpace===Wa?"display-p3":"srgb"}get outputEncoding(){return console.warn("THREE.WebGLRenderer: Property .outputEncoding has been removed. Use .outputColorSpace instead."),this.outputColorSpace===Ze?cr:ep}set outputEncoding(t){console.warn("THREE.WebGLRenderer: Property .outputEncoding has been removed. Use .outputColorSpace instead."),this.outputColorSpace=t===cr?Ze:di}get useLegacyLights(){return console.warn("THREE.WebGLRenderer: The property .useLegacyLights has been deprecated. Migrate your lighting according to the following guide: https://discourse.threejs.org/t/updates-to-lighting-in-three-js-r155/53733."),this._useLegacyLights}set useLegacyLights(t){console.warn("THREE.WebGLRenderer: The property .useLegacyLights has been deprecated. Migrate your lighting according to the following guide: https://discourse.threejs.org/t/updates-to-lighting-in-three-js-r155/53733."),this._useLegacyLights=t}}class Cx extends Sp{}Cx.prototype.isWebGL1Renderer=!0;class Wl{constructor(t,e=25e-5){this.isFogExp2=!0,this.name="",this.color=new oe(t),this.density=e}clone(){return new Wl(this.color,this.density)}toJSON(){return{type:"FogExp2",name:this.name,color:this.color.getHex(),density:this.density}}}let Ix=class extends je{constructor(){super(),this.isScene=!0,this.type="Scene",this.background=null,this.environment=null,this.fog=null,this.backgroundBlurriness=0,this.backgroundIntensity=1,this.overrideMaterial=null,typeof __THREE_DEVTOOLS__<"u"&&__THREE_DEVTOOLS__.dispatchEvent(new CustomEvent("observe",{detail:this}))}copy(t,e){return super.copy(t,e),t.background!==null&&(this.background=t.background.clone()),t.environment!==null&&(this.environment=t.environment.clone()),t.fog!==null&&(this.fog=t.fog.clone()),this.backgroundBlurriness=t.backgroundBlurriness,this.backgroundIntensity=t.backgroundIntensity,t.overrideMaterial!==null&&(this.overrideMaterial=t.overrideMaterial.clone()),this.matrixAutoUpdate=t.matrixAutoUpdate,this}toJSON(t){const e=super.toJSON(t);return this.fog!==null&&(e.object.fog=this.fog.toJSON()),this.backgroundBlurriness>0&&(e.object.backgroundBlurriness=this.backgroundBlurriness),this.backgroundIntensity!==1&&(e.object.backgroundIntensity=this.backgroundIntensity),e}};class Px{constructor(t,e){this.isInterleavedBuffer=!0,this.array=t,this.stride=e,this.count=t!==void 0?t.length/e:0,this.usage=hl,this._updateRange={offset:0,count:-1},this.updateRanges=[],this.version=0,this.uuid=Ui()}onUploadCallback(){}set needsUpdate(t){t===!0&&this.version++}get updateRange(){return console.warn("THREE.InterleavedBuffer: updateRange() is deprecated and will be removed in r169. Use addUpdateRange() instead."),this._updateRange}setUsage(t){return this.usage=t,this}addUpdateRange(t,e){this.updateRanges.push({start:t,count:e})}clearUpdateRanges(){this.updateRanges.length=0}copy(t){return this.array=new t.array.constructor(t.array),this.count=t.count,this.stride=t.stride,this.usage=t.usage,this}copyAt(t,e,n){t*=this.stride,n*=e.stride;for(let r=0,s=this.stride;r<s;r++)this.array[t+r]=e.array[n+r];return this}set(t,e=0){return this.array.set(t,e),this}clone(t){t.arrayBuffers===void 0&&(t.arrayBuffers={}),this.array.buffer._uuid===void 0&&(this.array.buffer._uuid=Ui()),t.arrayBuffers[this.array.buffer._uuid]===void 0&&(t.arrayBuffers[this.array.buffer._uuid]=this.array.slice(0).buffer);const e=new this.array.constructor(t.arrayBuffers[this.array.buffer._uuid]),n=new this.constructor(e,this.stride);return n.setUsage(this.usage),n}onUpload(t){return this.onUploadCallback=t,this}toJSON(t){return t.arrayBuffers===void 0&&(t.arrayBuffers={}),this.array.buffer._uuid===void 0&&(this.array.buffer._uuid=Ui()),t.arrayBuffers[this.array.buffer._uuid]===void 0&&(t.arrayBuffers[this.array.buffer._uuid]=Array.from(new Uint32Array(this.array.buffer))),{uuid:this.uuid,buffer:this.array.buffer._uuid,type:this.array.constructor.name,stride:this.stride}}}const dn=new k;class Aa{constructor(t,e,n,r=!1){this.isInterleavedBufferAttribute=!0,this.name="",this.data=t,this.itemSize=e,this.offset=n,this.normalized=r}get count(){return this.data.count}get array(){return this.data.array}set needsUpdate(t){this.data.needsUpdate=t}applyMatrix4(t){for(let e=0,n=this.data.count;e<n;e++)dn.fromBufferAttribute(this,e),dn.applyMatrix4(t),this.setXYZ(e,dn.x,dn.y,dn.z);return this}applyNormalMatrix(t){for(let e=0,n=this.count;e<n;e++)dn.fromBufferAttribute(this,e),dn.applyNormalMatrix(t),this.setXYZ(e,dn.x,dn.y,dn.z);return this}transformDirection(t){for(let e=0,n=this.count;e<n;e++)dn.fromBufferAttribute(this,e),dn.transformDirection(t),this.setXYZ(e,dn.x,dn.y,dn.z);return this}setX(t,e){return this.normalized&&(e=xe(e,this.array)),this.data.array[t*this.data.stride+this.offset]=e,this}setY(t,e){return this.normalized&&(e=xe(e,this.array)),this.data.array[t*this.data.stride+this.offset+1]=e,this}setZ(t,e){return this.normalized&&(e=xe(e,this.array)),this.data.array[t*this.data.stride+this.offset+2]=e,this}setW(t,e){return this.normalized&&(e=xe(e,this.array)),this.data.array[t*this.data.stride+this.offset+3]=e,this}getX(t){let e=this.data.array[t*this.data.stride+this.offset];return this.normalized&&(e=ai(e,this.array)),e}getY(t){let e=this.data.array[t*this.data.stride+this.offset+1];return this.normalized&&(e=ai(e,this.array)),e}getZ(t){let e=this.data.array[t*this.data.stride+this.offset+2];return this.normalized&&(e=ai(e,this.array)),e}getW(t){let e=this.data.array[t*this.data.stride+this.offset+3];return this.normalized&&(e=ai(e,this.array)),e}setXY(t,e,n){return t=t*this.data.stride+this.offset,this.normalized&&(e=xe(e,this.array),n=xe(n,this.array)),this.data.array[t+0]=e,this.data.array[t+1]=n,this}setXYZ(t,e,n,r){return t=t*this.data.stride+this.offset,this.normalized&&(e=xe(e,this.array),n=xe(n,this.array),r=xe(r,this.array)),this.data.array[t+0]=e,this.data.array[t+1]=n,this.data.array[t+2]=r,this}setXYZW(t,e,n,r,s){return t=t*this.data.stride+this.offset,this.normalized&&(e=xe(e,this.array),n=xe(n,this.array),r=xe(r,this.array),s=xe(s,this.array)),this.data.array[t+0]=e,this.data.array[t+1]=n,this.data.array[t+2]=r,this.data.array[t+3]=s,this}clone(t){if(t===void 0){console.log("THREE.InterleavedBufferAttribute.clone(): Cloning an interleaved buffer attribute will de-interleave buffer data.");const e=[];for(let n=0;n<this.count;n++){const r=n*this.data.stride+this.offset;for(let s=0;s<this.itemSize;s++)e.push(this.data.array[r+s])}return new xn(new this.array.constructor(e),this.itemSize,this.normalized)}else return t.interleavedBuffers===void 0&&(t.interleavedBuffers={}),t.interleavedBuffers[this.data.uuid]===void 0&&(t.interleavedBuffers[this.data.uuid]=this.data.clone(t)),new Aa(t.interleavedBuffers[this.data.uuid],this.itemSize,this.offset,this.normalized)}toJSON(t){if(t===void 0){console.log("THREE.InterleavedBufferAttribute.toJSON(): Serializing an interleaved buffer attribute will de-interleave buffer data.");const e=[];for(let n=0;n<this.count;n++){const r=n*this.data.stride+this.offset;for(let s=0;s<this.itemSize;s++)e.push(this.data.array[r+s])}return{itemSize:this.itemSize,type:this.array.constructor.name,array:e,normalized:this.normalized}}else return t.interleavedBuffers===void 0&&(t.interleavedBuffers={}),t.interleavedBuffers[this.data.uuid]===void 0&&(t.interleavedBuffers[this.data.uuid]=this.data.toJSON(t)),{isInterleavedBufferAttribute:!0,itemSize:this.itemSize,data:this.data.uuid,offset:this.offset,normalized:this.normalized}}}class Tp extends gs{constructor(t){super(),this.isSpriteMaterial=!0,this.type="SpriteMaterial",this.color=new oe(16777215),this.map=null,this.alphaMap=null,this.rotation=0,this.sizeAttenuation=!0,this.transparent=!0,this.fog=!0,this.setValues(t)}copy(t){return super.copy(t),this.color.copy(t.color),this.map=t.map,this.alphaMap=t.alphaMap,this.rotation=t.rotation,this.sizeAttenuation=t.sizeAttenuation,this.fog=t.fog,this}}let Gr;const ks=new k,Wr=new k,qr=new k,Xr=new Vt,Hs=new Vt,Mp=new Be,ea=new k,zs=new k,na=new k,Td=new Vt,zc=new Vt,Md=new Vt;class Dx extends je{constructor(t=new Tp){if(super(),this.isSprite=!0,this.type="Sprite",Gr===void 0){Gr=new yn;const e=new Float32Array([-.5,-.5,0,0,0,.5,-.5,0,1,0,.5,.5,0,1,1,-.5,.5,0,0,1]),n=new Px(e,5);Gr.setIndex([0,1,2,0,2,3]),Gr.setAttribute("position",new Aa(n,3,0,!1)),Gr.setAttribute("uv",new Aa(n,2,3,!1))}this.geometry=Gr,this.material=t,this.center=new Vt(.5,.5)}raycast(t,e){t.camera===null&&console.error('THREE.Sprite: "Raycaster.camera" needs to be set in order to raycast against sprites.'),Wr.setFromMatrixScale(this.matrixWorld),Mp.copy(t.camera.matrixWorld),this.modelViewMatrix.multiplyMatrices(t.camera.matrixWorldInverse,this.matrixWorld),qr.setFromMatrixPosition(this.modelViewMatrix),t.camera.isPerspectiveCamera&&this.material.sizeAttenuation===!1&&Wr.multiplyScalar(-qr.z);const n=this.material.rotation;let r,s;n!==0&&(s=Math.cos(n),r=Math.sin(n));const o=this.center;ia(ea.set(-.5,-.5,0),qr,o,Wr,r,s),ia(zs.set(.5,-.5,0),qr,o,Wr,r,s),ia(na.set(.5,.5,0),qr,o,Wr,r,s),Td.set(0,0),zc.set(1,0),Md.set(1,1);let a=t.ray.intersectTriangle(ea,zs,na,!1,ks);if(a===null&&(ia(zs.set(-.5,.5,0),qr,o,Wr,r,s),zc.set(0,1),a=t.ray.intersectTriangle(ea,na,zs,!1,ks),a===null))return;const c=t.ray.origin.distanceTo(ks);c<t.near||c>t.far||e.push({distance:c,point:ks.clone(),uv:Dn.getInterpolation(ks,ea,zs,na,Td,zc,Md,new Vt),face:null,object:this})}copy(t,e){return super.copy(t,e),t.center!==void 0&&this.center.copy(t.center),this.material=t.material,this}}function ia(i,t,e,n,r,s){Xr.subVectors(i,e).addScalar(.5).multiply(n),r!==void 0?(Hs.x=s*Xr.x-r*Xr.y,Hs.y=r*Xr.x+s*Xr.y):Hs.copy(Xr),i.copy(t),i.x+=Hs.x,i.y+=Hs.y,i.applyMatrix4(Mp)}class Lx extends En{constructor(t,e,n,r,s,o,a,c,h){super(t,e,n,r,s,o,a,c,h),this.isCanvasTexture=!0,this.needsUpdate=!0}}class Ra extends yn{constructor(t=1,e=32,n=0,r=Math.PI*2){super(),this.type="CircleGeometry",this.parameters={radius:t,segments:e,thetaStart:n,thetaLength:r},e=Math.max(3,e);const s=[],o=[],a=[],c=[],h=new k,u=new Vt;o.push(0,0,0),a.push(0,0,1),c.push(.5,.5);for(let d=0,f=3;d<=e;d++,f+=3){const p=n+d/e*r;h.x=t*Math.cos(p),h.y=t*Math.sin(p),o.push(h.x,h.y,h.z),a.push(0,0,1),u.x=(o[f]/t+1)/2,u.y=(o[f+1]/t+1)/2,c.push(u.x,u.y)}for(let d=1;d<=e;d++)s.push(d,d+1,0);this.setIndex(s),this.setAttribute("position",new Re(o,3)),this.setAttribute("normal",new Re(a,3)),this.setAttribute("uv",new Re(c,2))}copy(t){return super.copy(t),this.parameters=Object.assign({},t.parameters),this}static fromJSON(t){return new Ra(t.radius,t.segments,t.thetaStart,t.thetaLength)}}class ja extends yn{constructor(t=1,e=1,n=1,r=32,s=1,o=!1,a=0,c=Math.PI*2){super(),this.type="CylinderGeometry",this.parameters={radiusTop:t,radiusBottom:e,height:n,radialSegments:r,heightSegments:s,openEnded:o,thetaStart:a,thetaLength:c};const h=this;r=Math.floor(r),s=Math.floor(s);const u=[],d=[],f=[],p=[];let y=0;const E=[],_=n/2;let m=0;C(),o===!1&&(t>0&&w(!0),e>0&&w(!1)),this.setIndex(u),this.setAttribute("position",new Re(d,3)),this.setAttribute("normal",new Re(f,3)),this.setAttribute("uv",new Re(p,2));function C(){const b=new k,N=new k;let U=0;const T=(e-t)/n;for(let M=0;M<=s;M++){const v=[],S=M/s,P=S*(e-t)+t;for(let L=0;L<=r;L++){const R=L/r,V=R*c+a,G=Math.sin(V),K=Math.cos(V);N.x=P*G,N.y=-S*n+_,N.z=P*K,d.push(N.x,N.y,N.z),b.set(G,T,K).normalize(),f.push(b.x,b.y,b.z),p.push(R,1-S),v.push(y++)}E.push(v)}for(let M=0;M<r;M++)for(let v=0;v<s;v++){const S=E[v][M],P=E[v+1][M],L=E[v+1][M+1],R=E[v][M+1];u.push(S,P,R),u.push(P,L,R),U+=6}h.addGroup(m,U,0),m+=U}function w(b){const N=y,U=new Vt,T=new k;let M=0;const v=b===!0?t:e,S=b===!0?1:-1;for(let L=1;L<=r;L++)d.push(0,_*S,0),f.push(0,S,0),p.push(.5,.5),y++;const P=y;for(let L=0;L<=r;L++){const V=L/r*c+a,G=Math.cos(V),K=Math.sin(V);T.x=v*K,T.y=_*S,T.z=v*G,d.push(T.x,T.y,T.z),f.push(0,S,0),U.x=G*.5+.5,U.y=K*.5*S+.5,p.push(U.x,U.y),y++}for(let L=0;L<r;L++){const R=N+L,V=P+L;b===!0?u.push(V,V+1,R):u.push(V+1,V,R),M+=3}h.addGroup(m,M,b===!0?1:2),m+=M}}copy(t){return super.copy(t),this.parameters=Object.assign({},t.parameters),this}static fromJSON(t){return new ja(t.radiusTop,t.radiusBottom,t.height,t.radialSegments,t.heightSegments,t.openEnded,t.thetaStart,t.thetaLength)}}class mo extends yn{constructor(t=.5,e=1,n=32,r=1,s=0,o=Math.PI*2){super(),this.type="RingGeometry",this.parameters={innerRadius:t,outerRadius:e,thetaSegments:n,phiSegments:r,thetaStart:s,thetaLength:o},n=Math.max(3,n),r=Math.max(1,r);const a=[],c=[],h=[],u=[];let d=t;const f=(e-t)/r,p=new k,y=new Vt;for(let E=0;E<=r;E++){for(let _=0;_<=n;_++){const m=s+_/n*o;p.x=d*Math.cos(m),p.y=d*Math.sin(m),c.push(p.x,p.y,p.z),h.push(0,0,1),y.x=(p.x/e+1)/2,y.y=(p.y/e+1)/2,u.push(y.x,y.y)}d+=f}for(let E=0;E<r;E++){const _=E*(n+1);for(let m=0;m<n;m++){const C=m+_,w=C,b=C+n+1,N=C+n+2,U=C+1;a.push(w,b,U),a.push(b,N,U)}}this.setIndex(a),this.setAttribute("position",new Re(c,3)),this.setAttribute("normal",new Re(h,3)),this.setAttribute("uv",new Re(u,2))}copy(t){return super.copy(t),this.parameters=Object.assign({},t.parameters),this}static fromJSON(t){return new mo(t.innerRadius,t.outerRadius,t.thetaSegments,t.phiSegments,t.thetaStart,t.thetaLength)}}class ql extends yn{constructor(t=1,e=32,n=16,r=0,s=Math.PI*2,o=0,a=Math.PI){super(),this.type="SphereGeometry",this.parameters={radius:t,widthSegments:e,heightSegments:n,phiStart:r,phiLength:s,thetaStart:o,thetaLength:a},e=Math.max(3,Math.floor(e)),n=Math.max(2,Math.floor(n));const c=Math.min(o+a,Math.PI);let h=0;const u=[],d=new k,f=new k,p=[],y=[],E=[],_=[];for(let m=0;m<=n;m++){const C=[],w=m/n;let b=0;m===0&&o===0?b=.5/e:m===n&&c===Math.PI&&(b=-.5/e);for(let N=0;N<=e;N++){const U=N/e;d.x=-t*Math.cos(r+U*s)*Math.sin(o+w*a),d.y=t*Math.cos(o+w*a),d.z=t*Math.sin(r+U*s)*Math.sin(o+w*a),y.push(d.x,d.y,d.z),f.copy(d).normalize(),E.push(f.x,f.y,f.z),_.push(U+b,1-w),C.push(h++)}u.push(C)}for(let m=0;m<n;m++)for(let C=0;C<e;C++){const w=u[m][C+1],b=u[m][C],N=u[m+1][C],U=u[m+1][C+1];(m!==0||o>0)&&p.push(w,b,U),(m!==n-1||c<Math.PI)&&p.push(b,N,U)}this.setIndex(p),this.setAttribute("position",new Re(y,3)),this.setAttribute("normal",new Re(E,3)),this.setAttribute("uv",new Re(_,2))}copy(t){return super.copy(t),this.parameters=Object.assign({},t.parameters),this}static fromJSON(t){return new ql(t.radius,t.widthSegments,t.heightSegments,t.phiStart,t.phiLength,t.thetaStart,t.thetaLength)}}class Ya extends yn{constructor(t=1,e=.4,n=12,r=48,s=Math.PI*2){super(),this.type="TorusGeometry",this.parameters={radius:t,tube:e,radialSegments:n,tubularSegments:r,arc:s},n=Math.floor(n),r=Math.floor(r);const o=[],a=[],c=[],h=[],u=new k,d=new k,f=new k;for(let p=0;p<=n;p++)for(let y=0;y<=r;y++){const E=y/r*s,_=p/n*Math.PI*2;d.x=(t+e*Math.cos(_))*Math.cos(E),d.y=(t+e*Math.cos(_))*Math.sin(E),d.z=e*Math.sin(_),a.push(d.x,d.y,d.z),u.x=t*Math.cos(E),u.y=t*Math.sin(E),f.subVectors(d,u).normalize(),c.push(f.x,f.y,f.z),h.push(y/r),h.push(p/n)}for(let p=1;p<=n;p++)for(let y=1;y<=r;y++){const E=(r+1)*p+y-1,_=(r+1)*(p-1)+y-1,m=(r+1)*(p-1)+y,C=(r+1)*p+y;o.push(E,_,C),o.push(_,m,C)}this.setIndex(o),this.setAttribute("position",new Re(a,3)),this.setAttribute("normal",new Re(c,3)),this.setAttribute("uv",new Re(h,2))}copy(t){return super.copy(t),this.parameters=Object.assign({},t.parameters),this}static fromJSON(t){return new Ya(t.radius,t.tube,t.radialSegments,t.tubularSegments,t.arc)}}class wp extends gs{constructor(t){super(),this.isMeshStandardMaterial=!0,this.defines={STANDARD:""},this.type="MeshStandardMaterial",this.color=new oe(16777215),this.roughness=1,this.metalness=0,this.map=null,this.lightMap=null,this.lightMapIntensity=1,this.aoMap=null,this.aoMapIntensity=1,this.emissive=new oe(0),this.emissiveIntensity=1,this.emissiveMap=null,this.bumpMap=null,this.bumpScale=1,this.normalMap=null,this.normalMapType=np,this.normalScale=new Vt(1,1),this.displacementMap=null,this.displacementScale=1,this.displacementBias=0,this.roughnessMap=null,this.metalnessMap=null,this.alphaMap=null,this.envMap=null,this.envMapIntensity=1,this.wireframe=!1,this.wireframeLinewidth=1,this.wireframeLinecap="round",this.wireframeLinejoin="round",this.flatShading=!1,this.fog=!0,this.setValues(t)}copy(t){return super.copy(t),this.defines={STANDARD:""},this.color.copy(t.color),this.roughness=t.roughness,this.metalness=t.metalness,this.map=t.map,this.lightMap=t.lightMap,this.lightMapIntensity=t.lightMapIntensity,this.aoMap=t.aoMap,this.aoMapIntensity=t.aoMapIntensity,this.emissive.copy(t.emissive),this.emissiveMap=t.emissiveMap,this.emissiveIntensity=t.emissiveIntensity,this.bumpMap=t.bumpMap,this.bumpScale=t.bumpScale,this.normalMap=t.normalMap,this.normalMapType=t.normalMapType,this.normalScale.copy(t.normalScale),this.displacementMap=t.displacementMap,this.displacementScale=t.displacementScale,this.displacementBias=t.displacementBias,this.roughnessMap=t.roughnessMap,this.metalnessMap=t.metalnessMap,this.alphaMap=t.alphaMap,this.envMap=t.envMap,this.envMapIntensity=t.envMapIntensity,this.wireframe=t.wireframe,this.wireframeLinewidth=t.wireframeLinewidth,this.wireframeLinecap=t.wireframeLinecap,this.wireframeLinejoin=t.wireframeLinejoin,this.flatShading=t.flatShading,this.fog=t.fog,this}}class Xl extends je{constructor(t,e=1){super(),this.isLight=!0,this.type="Light",this.color=new oe(t),this.intensity=e}dispose(){}copy(t,e){return super.copy(t,e),this.color.copy(t.color),this.intensity=t.intensity,this}toJSON(t){const e=super.toJSON(t);return e.object.color=this.color.getHex(),e.object.intensity=this.intensity,this.groundColor!==void 0&&(e.object.groundColor=this.groundColor.getHex()),this.distance!==void 0&&(e.object.distance=this.distance),this.angle!==void 0&&(e.object.angle=this.angle),this.decay!==void 0&&(e.object.decay=this.decay),this.penumbra!==void 0&&(e.object.penumbra=this.penumbra),this.shadow!==void 0&&(e.object.shadow=this.shadow.toJSON()),e}}class Nx extends Xl{constructor(t,e,n){super(t,n),this.isHemisphereLight=!0,this.type="HemisphereLight",this.position.copy(je.DEFAULT_UP),this.updateMatrix(),this.groundColor=new oe(e)}copy(t,e){return super.copy(t,e),this.groundColor.copy(t.groundColor),this}}const Gc=new Be,wd=new k,bd=new k;class Ux{constructor(t){this.camera=t,this.bias=0,this.normalBias=0,this.radius=1,this.blurSamples=8,this.mapSize=new Vt(512,512),this.map=null,this.mapPass=null,this.matrix=new Be,this.autoUpdate=!0,this.needsUpdate=!1,this._frustum=new zl,this._frameExtents=new Vt(1,1),this._viewportCount=1,this._viewports=[new Xe(0,0,1,1)]}getViewportCount(){return this._viewportCount}getFrustum(){return this._frustum}updateMatrices(t){const e=this.camera,n=this.matrix;wd.setFromMatrixPosition(t.matrixWorld),e.position.copy(wd),bd.setFromMatrixPosition(t.target.matrixWorld),e.lookAt(bd),e.updateMatrixWorld(),Gc.multiplyMatrices(e.projectionMatrix,e.matrixWorldInverse),this._frustum.setFromProjectionMatrix(Gc),n.set(.5,0,0,.5,0,.5,0,.5,0,0,.5,.5,0,0,0,1),n.multiply(Gc)}getViewport(t){return this._viewports[t]}getFrameExtents(){return this._frameExtents}dispose(){this.map&&this.map.dispose(),this.mapPass&&this.mapPass.dispose()}copy(t){return this.camera=t.camera.clone(),this.bias=t.bias,this.radius=t.radius,this.mapSize.copy(t.mapSize),this}clone(){return new this.constructor().copy(this)}toJSON(){const t={};return this.bias!==0&&(t.bias=this.bias),this.normalBias!==0&&(t.normalBias=this.normalBias),this.radius!==1&&(t.radius=this.radius),(this.mapSize.x!==512||this.mapSize.y!==512)&&(t.mapSize=this.mapSize.toArray()),t.camera=this.camera.toJSON(!1).object,delete t.camera.matrix,t}}class Ox extends Ux{constructor(){super(new mp(-5,5,5,-5,.5,500)),this.isDirectionalLightShadow=!0}}class Ad extends Xl{constructor(t,e){super(t,e),this.isDirectionalLight=!0,this.type="DirectionalLight",this.position.copy(je.DEFAULT_UP),this.updateMatrix(),this.target=new je,this.shadow=new Ox}dispose(){this.shadow.dispose()}copy(t){return super.copy(t),this.target=t.target.clone(),this.shadow=t.shadow.clone(),this}}class Fx extends Xl{constructor(t,e){super(t,e),this.isAmbientLight=!0,this.type="AmbientLight"}}class bp{constructor(t,e,n=0,r=1/0){this.ray=new kl(t,e),this.near=n,this.far=r,this.camera=null,this.layers=new Hl,this.params={Mesh:{},Line:{threshold:1},LOD:{},Points:{threshold:1},Sprite:{}}}set(t,e){this.ray.set(t,e)}setFromCamera(t,e){e.isPerspectiveCamera?(this.ray.origin.setFromMatrixPosition(e.matrixWorld),this.ray.direction.set(t.x,t.y,.5).unproject(e).sub(this.ray.origin).normalize(),this.camera=e):e.isOrthographicCamera?(this.ray.origin.set(t.x,t.y,(e.near+e.far)/(e.near-e.far)).unproject(e),this.ray.direction.set(0,0,-1).transformDirection(e.matrixWorld),this.camera=e):console.error("THREE.Raycaster: Unsupported camera type: "+e.type)}intersectObject(t,e=!0,n=[]){return ml(t,this,n,e),n.sort(Rd),n}intersectObjects(t,e=!0,n=[]){for(let r=0,s=t.length;r<s;r++)ml(t[r],this,n,e);return n.sort(Rd),n}}function Rd(i,t){return i.distance-t.distance}function ml(i,t,e,n){if(i.layers.test(t.layers)&&i.raycast(t,e),n===!0){const r=i.children;for(let s=0,o=r.length;s<o;s++)ml(r[s],t,e,!0)}}class Cd{constructor(t=1,e=0,n=0){return this.radius=t,this.phi=e,this.theta=n,this}set(t,e,n){return this.radius=t,this.phi=e,this.theta=n,this}copy(t){return this.radius=t.radius,this.phi=t.phi,this.theta=t.theta,this}makeSafe(){return this.phi=Math.max(1e-6,Math.min(Math.PI-1e-6,this.phi)),this}setFromVector3(t){return this.setFromCartesianCoords(t.x,t.y,t.z)}setFromCartesianCoords(t,e,n){return this.radius=Math.sqrt(t*t+e*e+n*n),this.radius===0?(this.theta=0,this.phi=0):(this.theta=Math.atan2(t,n),this.phi=Math.acos(mn(e/this.radius,-1,1))),this}clone(){return new this.constructor().copy(this)}}typeof __THREE_DEVTOOLS__<"u"&&__THREE_DEVTOOLS__.dispatchEvent(new CustomEvent("register",{detail:{revision:Ol}}));typeof window<"u"&&(window.__THREE__?console.warn("WARNING: Multiple instances of Three.js being imported."):window.__THREE__=Ol);const Id={type:"change"},Wc={type:"start"},Pd={type:"end"},ra=new kl,Dd=new bi,Vx=Math.cos(70*T_.DEG2RAD);class Bx extends gr{constructor(t,e){super(),this.object=t,this.domElement=e,this.domElement.style.touchAction="none",this.enabled=!0,this.target=new k,this.cursor=new k,this.minDistance=0,this.maxDistance=1/0,this.minZoom=0,this.maxZoom=1/0,this.minTargetRadius=0,this.maxTargetRadius=1/0,this.minPolarAngle=0,this.maxPolarAngle=Math.PI,this.minAzimuthAngle=-1/0,this.maxAzimuthAngle=1/0,this.enableDamping=!1,this.dampingFactor=.05,this.enableZoom=!0,this.zoomSpeed=1,this.enableRotate=!0,this.rotateSpeed=1,this.enablePan=!0,this.panSpeed=1,this.screenSpacePanning=!0,this.keyPanSpeed=7,this.zoomToCursor=!1,this.autoRotate=!1,this.autoRotateSpeed=2,this.keys={LEFT:"ArrowLeft",UP:"ArrowUp",RIGHT:"ArrowRight",BOTTOM:"ArrowDown"},this.mouseButtons={LEFT:wi.ROTATE,MIDDLE:wi.DOLLY,RIGHT:wi.PAN},this.touches={ONE:br.ROTATE,TWO:br.DOLLY_PAN},this.target0=this.target.clone(),this.position0=this.object.position.clone(),this.zoom0=this.object.zoom,this._domElementKeyEvents=null,this.getPolarAngle=function(){return a.phi},this.getAzimuthalAngle=function(){return a.theta},this.getDistance=function(){return this.object.position.distanceTo(this.target)},this.listenToKeyEvents=function(F){F.addEventListener("keydown",Nt),this._domElementKeyEvents=F},this.stopListenToKeyEvents=function(){this._domElementKeyEvents.removeEventListener("keydown",Nt),this._domElementKeyEvents=null},this.saveState=function(){n.target0.copy(n.target),n.position0.copy(n.object.position),n.zoom0=n.object.zoom},this.reset=function(){n.target.copy(n.target0),n.object.position.copy(n.position0),n.object.zoom=n.zoom0,n.object.updateProjectionMatrix(),n.dispatchEvent(Id),n.update(),s=r.NONE},this.update=function(){const F=new k,dt=new ur().setFromUnitVectors(t.up,new k(0,1,0)),It=dt.clone().invert(),St=new k,at=new ur,B=new k,ft=2*Math.PI;return function(Ut=null){const Ot=n.object.position;F.copy(Ot).sub(n.target),F.applyQuaternion(dt),a.setFromVector3(F),n.autoRotate&&s===r.NONE&&L(S(Ut)),n.enableDamping?(a.theta+=c.theta*n.dampingFactor,a.phi+=c.phi*n.dampingFactor):(a.theta+=c.theta,a.phi+=c.phi);let ne=n.minAzimuthAngle,se=n.maxAzimuthAngle;isFinite(ne)&&isFinite(se)&&(ne<-Math.PI?ne+=ft:ne>Math.PI&&(ne-=ft),se<-Math.PI?se+=ft:se>Math.PI&&(se-=ft),ne<=se?a.theta=Math.max(ne,Math.min(se,a.theta)):a.theta=a.theta>(ne+se)/2?Math.max(ne,a.theta):Math.min(se,a.theta)),a.phi=Math.max(n.minPolarAngle,Math.min(n.maxPolarAngle,a.phi)),a.makeSafe(),n.enableDamping===!0?n.target.addScaledVector(u,n.dampingFactor):n.target.add(u),n.target.sub(n.cursor),n.target.clampLength(n.minTargetRadius,n.maxTargetRadius),n.target.add(n.cursor),n.zoomToCursor&&U||n.object.isOrthographicCamera?a.radius=tt(a.radius):a.radius=tt(a.radius*h),F.setFromSpherical(a),F.applyQuaternion(It),Ot.copy(n.target).add(F),n.object.lookAt(n.target),n.enableDamping===!0?(c.theta*=1-n.dampingFactor,c.phi*=1-n.dampingFactor,u.multiplyScalar(1-n.dampingFactor)):(c.set(0,0,0),u.set(0,0,0));let _e=!1;if(n.zoomToCursor&&U){let Ae=null;if(n.object.isPerspectiveCamera){const Xt=F.length();Ae=tt(Xt*h);const Ce=Xt-Ae;n.object.position.addScaledVector(b,Ce),n.object.updateMatrixWorld()}else if(n.object.isOrthographicCamera){const Xt=new k(N.x,N.y,0);Xt.unproject(n.object),n.object.zoom=Math.max(n.minZoom,Math.min(n.maxZoom,n.object.zoom/h)),n.object.updateProjectionMatrix(),_e=!0;const Ce=new k(N.x,N.y,0);Ce.unproject(n.object),n.object.position.sub(Ce).add(Xt),n.object.updateMatrixWorld(),Ae=F.length()}else console.warn("WARNING: OrbitControls.js encountered an unknown camera type - zoom to cursor disabled."),n.zoomToCursor=!1;Ae!==null&&(this.screenSpacePanning?n.target.set(0,0,-1).transformDirection(n.object.matrix).multiplyScalar(Ae).add(n.object.position):(ra.origin.copy(n.object.position),ra.direction.set(0,0,-1).transformDirection(n.object.matrix),Math.abs(n.object.up.dot(ra.direction))<Vx?t.lookAt(n.target):(Dd.setFromNormalAndCoplanarPoint(n.object.up,n.target),ra.intersectPlane(Dd,n.target))))}else n.object.isOrthographicCamera&&(n.object.zoom=Math.max(n.minZoom,Math.min(n.maxZoom,n.object.zoom/h)),n.object.updateProjectionMatrix(),_e=!0);return h=1,U=!1,_e||St.distanceToSquared(n.object.position)>o||8*(1-at.dot(n.object.quaternion))>o||B.distanceToSquared(n.target)>0?(n.dispatchEvent(Id),St.copy(n.object.position),at.copy(n.object.quaternion),B.copy(n.target),!0):!1}}(),this.dispose=function(){n.domElement.removeEventListener("contextmenu",he),n.domElement.removeEventListener("pointerdown",O),n.domElement.removeEventListener("pointercancel",q),n.domElement.removeEventListener("wheel",ct),n.domElement.removeEventListener("pointermove",A),n.domElement.removeEventListener("pointerup",q),n._domElementKeyEvents!==null&&(n._domElementKeyEvents.removeEventListener("keydown",Nt),n._domElementKeyEvents=null)};const n=this,r={NONE:-1,ROTATE:0,DOLLY:1,PAN:2,TOUCH_ROTATE:3,TOUCH_PAN:4,TOUCH_DOLLY_PAN:5,TOUCH_DOLLY_ROTATE:6};let s=r.NONE;const o=1e-6,a=new Cd,c=new Cd;let h=1;const u=new k,d=new Vt,f=new Vt,p=new Vt,y=new Vt,E=new Vt,_=new Vt,m=new Vt,C=new Vt,w=new Vt,b=new k,N=new Vt;let U=!1;const T=[],M={};let v=!1;function S(F){return F!==null?2*Math.PI/60*n.autoRotateSpeed*F:2*Math.PI/60/60*n.autoRotateSpeed}function P(F){const dt=Math.abs(F*.01);return Math.pow(.95,n.zoomSpeed*dt)}function L(F){c.theta-=F}function R(F){c.phi-=F}const V=function(){const F=new k;return function(It,St){F.setFromMatrixColumn(St,0),F.multiplyScalar(-It),u.add(F)}}(),G=function(){const F=new k;return function(It,St){n.screenSpacePanning===!0?F.setFromMatrixColumn(St,1):(F.setFromMatrixColumn(St,0),F.crossVectors(n.object.up,F)),F.multiplyScalar(It),u.add(F)}}(),K=function(){const F=new k;return function(It,St){const at=n.domElement;if(n.object.isPerspectiveCamera){const B=n.object.position;F.copy(B).sub(n.target);let ft=F.length();ft*=Math.tan(n.object.fov/2*Math.PI/180),V(2*It*ft/at.clientHeight,n.object.matrix),G(2*St*ft/at.clientHeight,n.object.matrix)}else n.object.isOrthographicCamera?(V(It*(n.object.right-n.object.left)/n.object.zoom/at.clientWidth,n.object.matrix),G(St*(n.object.top-n.object.bottom)/n.object.zoom/at.clientHeight,n.object.matrix)):(console.warn("WARNING: OrbitControls.js encountered an unknown camera type - pan disabled."),n.enablePan=!1)}}();function Z(F){n.object.isPerspectiveCamera||n.object.isOrthographicCamera?h/=F:(console.warn("WARNING: OrbitControls.js encountered an unknown camera type - dolly/zoom disabled."),n.enableZoom=!1)}function J(F){n.object.isPerspectiveCamera||n.object.isOrthographicCamera?h*=F:(console.warn("WARNING: OrbitControls.js encountered an unknown camera type - dolly/zoom disabled."),n.enableZoom=!1)}function et(F,dt){if(!n.zoomToCursor)return;U=!0;const It=n.domElement.getBoundingClientRect(),St=F-It.left,at=dt-It.top,B=It.width,ft=It.height;N.x=St/B*2-1,N.y=-(at/ft)*2+1,b.set(N.x,N.y,1).unproject(n.object).sub(n.object.position).normalize()}function tt(F){return Math.max(n.minDistance,Math.min(n.maxDistance,F))}function pt(F){d.set(F.clientX,F.clientY)}function ht(F){et(F.clientX,F.clientX),m.set(F.clientX,F.clientY)}function X(F){y.set(F.clientX,F.clientY)}function it(F){f.set(F.clientX,F.clientY),p.subVectors(f,d).multiplyScalar(n.rotateSpeed);const dt=n.domElement;L(2*Math.PI*p.x/dt.clientHeight),R(2*Math.PI*p.y/dt.clientHeight),d.copy(f),n.update()}function mt(F){C.set(F.clientX,F.clientY),w.subVectors(C,m),w.y>0?Z(P(w.y)):w.y<0&&J(P(w.y)),m.copy(C),n.update()}function Pt(F){E.set(F.clientX,F.clientY),_.subVectors(E,y).multiplyScalar(n.panSpeed),K(_.x,_.y),y.copy(E),n.update()}function Mt(F){et(F.clientX,F.clientY),F.deltaY<0?J(P(F.deltaY)):F.deltaY>0&&Z(P(F.deltaY)),n.update()}function Gt(F){let dt=!1;switch(F.code){case n.keys.UP:F.ctrlKey||F.metaKey||F.shiftKey?R(2*Math.PI*n.rotateSpeed/n.domElement.clientHeight):K(0,n.keyPanSpeed),dt=!0;break;case n.keys.BOTTOM:F.ctrlKey||F.metaKey||F.shiftKey?R(-2*Math.PI*n.rotateSpeed/n.domElement.clientHeight):K(0,-n.keyPanSpeed),dt=!0;break;case n.keys.LEFT:F.ctrlKey||F.metaKey||F.shiftKey?L(2*Math.PI*n.rotateSpeed/n.domElement.clientHeight):K(n.keyPanSpeed,0),dt=!0;break;case n.keys.RIGHT:F.ctrlKey||F.metaKey||F.shiftKey?L(-2*Math.PI*n.rotateSpeed/n.domElement.clientHeight):K(-n.keyPanSpeed,0),dt=!0;break}dt&&(F.preventDefault(),n.update())}function Ft(F){if(T.length===1)d.set(F.pageX,F.pageY);else{const dt=Et(F),It=.5*(F.pageX+dt.x),St=.5*(F.pageY+dt.y);d.set(It,St)}}function wt(F){if(T.length===1)y.set(F.pageX,F.pageY);else{const dt=Et(F),It=.5*(F.pageX+dt.x),St=.5*(F.pageY+dt.y);y.set(It,St)}}function Wt(F){const dt=Et(F),It=F.pageX-dt.x,St=F.pageY-dt.y,at=Math.sqrt(It*It+St*St);m.set(0,at)}function W(F){n.enableZoom&&Wt(F),n.enablePan&&wt(F)}function pe(F){n.enableZoom&&Wt(F),n.enableRotate&&Ft(F)}function Rt(F){if(T.length==1)f.set(F.pageX,F.pageY);else{const It=Et(F),St=.5*(F.pageX+It.x),at=.5*(F.pageY+It.y);f.set(St,at)}p.subVectors(f,d).multiplyScalar(n.rotateSpeed);const dt=n.domElement;L(2*Math.PI*p.x/dt.clientHeight),R(2*Math.PI*p.y/dt.clientHeight),d.copy(f)}function kt(F){if(T.length===1)E.set(F.pageX,F.pageY);else{const dt=Et(F),It=.5*(F.pageX+dt.x),St=.5*(F.pageY+dt.y);E.set(It,St)}_.subVectors(E,y).multiplyScalar(n.panSpeed),K(_.x,_.y),y.copy(E)}function bt(F){const dt=Et(F),It=F.pageX-dt.x,St=F.pageY-dt.y,at=Math.sqrt(It*It+St*St);C.set(0,at),w.set(0,Math.pow(C.y/m.y,n.zoomSpeed)),Z(w.y),m.copy(C);const B=(F.pageX+dt.x)*.5,ft=(F.pageY+dt.y)*.5;et(B,ft)}function ye(F){n.enableZoom&&bt(F),n.enablePan&&kt(F)}function te(F){n.enableZoom&&bt(F),n.enableRotate&&Rt(F)}function O(F){n.enabled!==!1&&(T.length===0&&(n.domElement.setPointerCapture(F.pointerId),n.domElement.addEventListener("pointermove",A),n.domElement.addEventListener("pointerup",q)),Kt(F),F.pointerType==="touch"?Qt(F):lt(F))}function A(F){n.enabled!==!1&&(F.pointerType==="touch"?st(F):rt(F))}function q(F){qt(F),T.length===0&&(n.domElement.releasePointerCapture(F.pointerId),n.domElement.removeEventListener("pointermove",A),n.domElement.removeEventListener("pointerup",q)),n.dispatchEvent(Pd),s=r.NONE}function lt(F){let dt;switch(F.button){case 0:dt=n.mouseButtons.LEFT;break;case 1:dt=n.mouseButtons.MIDDLE;break;case 2:dt=n.mouseButtons.RIGHT;break;default:dt=-1}switch(dt){case wi.DOLLY:if(n.enableZoom===!1)return;ht(F),s=r.DOLLY;break;case wi.ROTATE:if(F.ctrlKey||F.metaKey||F.shiftKey){if(n.enablePan===!1)return;X(F),s=r.PAN}else{if(n.enableRotate===!1)return;pt(F),s=r.ROTATE}break;case wi.PAN:if(F.ctrlKey||F.metaKey||F.shiftKey){if(n.enableRotate===!1)return;pt(F),s=r.ROTATE}else{if(n.enablePan===!1)return;X(F),s=r.PAN}break;default:s=r.NONE}s!==r.NONE&&n.dispatchEvent(Wc)}function rt(F){switch(s){case r.ROTATE:if(n.enableRotate===!1)return;it(F);break;case r.DOLLY:if(n.enableZoom===!1)return;mt(F);break;case r.PAN:if(n.enablePan===!1)return;Pt(F);break}}function ct(F){n.enabled===!1||n.enableZoom===!1||s!==r.NONE||(F.preventDefault(),n.dispatchEvent(Wc),Mt(Ct(F)),n.dispatchEvent(Pd))}function Ct(F){const dt=F.deltaMode,It={clientX:F.clientX,clientY:F.clientY,deltaY:F.deltaY};switch(dt){case 1:It.deltaY*=16;break;case 2:It.deltaY*=100;break}return F.ctrlKey&&!v&&(It.deltaY*=10),It}function ut(F){F.key==="Control"&&(v=!0,document.addEventListener("keyup",_t,{passive:!0,capture:!0}))}function _t(F){F.key==="Control"&&(v=!1,document.removeEventListener("keyup",_t,{passive:!0,capture:!0}))}function Nt(F){n.enabled===!1||n.enablePan===!1||Gt(F)}function Qt(F){switch(Lt(F),T.length){case 1:switch(n.touches.ONE){case br.ROTATE:if(n.enableRotate===!1)return;Ft(F),s=r.TOUCH_ROTATE;break;case br.PAN:if(n.enablePan===!1)return;wt(F),s=r.TOUCH_PAN;break;default:s=r.NONE}break;case 2:switch(n.touches.TWO){case br.DOLLY_PAN:if(n.enableZoom===!1&&n.enablePan===!1)return;W(F),s=r.TOUCH_DOLLY_PAN;break;case br.DOLLY_ROTATE:if(n.enableZoom===!1&&n.enableRotate===!1)return;pe(F),s=r.TOUCH_DOLLY_ROTATE;break;default:s=r.NONE}break;default:s=r.NONE}s!==r.NONE&&n.dispatchEvent(Wc)}function st(F){switch(Lt(F),s){case r.TOUCH_ROTATE:if(n.enableRotate===!1)return;Rt(F),n.update();break;case r.TOUCH_PAN:if(n.enablePan===!1)return;kt(F),n.update();break;case r.TOUCH_DOLLY_PAN:if(n.enableZoom===!1&&n.enablePan===!1)return;ye(F),n.update();break;case r.TOUCH_DOLLY_ROTATE:if(n.enableZoom===!1&&n.enableRotate===!1)return;te(F),n.update();break;default:s=r.NONE}}function he(F){n.enabled!==!1&&F.preventDefault()}function Kt(F){T.push(F.pointerId)}function qt(F){delete M[F.pointerId];for(let dt=0;dt<T.length;dt++)if(T[dt]==F.pointerId){T.splice(dt,1);return}}function Lt(F){let dt=M[F.pointerId];dt===void 0&&(dt=new Vt,M[F.pointerId]=dt),dt.set(F.pageX,F.pageY)}function Et(F){const dt=F.pointerId===T[0]?T[1]:T[0];return M[dt]}n.domElement.addEventListener("contextmenu",he),n.domElement.addEventListener("pointerdown",O),n.domElement.addEventListener("pointercancel",q),n.domElement.addEventListener("wheel",ct,{passive:!1}),document.addEventListener("keydown",ut,{passive:!0,capture:!0}),this.update()}}const Ii={LOW:"low",MEDIUM:"medium",HIGH:"high"},Ca={[Ii.LOW]:{gridSize:128,worldScale:16,maxSimSteps:2,label:"Low"},[Ii.MEDIUM]:{gridSize:192,worldScale:11,maxSimSteps:3,label:"Medium"},[Ii.HIGH]:{gridSize:256,worldScale:8,maxSimSteps:4,label:"High"}},gn={level:Ii.HIGH,gridWidth:256,gridHeight:256,gridSize:256*256,worldScale:8,maxSimSteps:4};function jl(i){const t=Ca[i];if(!t)return!1;gn.level=i,gn.gridWidth=t.gridSize,gn.gridHeight=t.gridSize,gn.gridSize=t.gridSize*t.gridSize,gn.worldScale=t.worldScale,gn.maxSimSteps=t.maxSimSteps;try{localStorage.setItem("drift-quality",i)}catch{}return!0}function kx(){try{const i=localStorage.getItem("drift-quality");if(i&&Ca[i])return jl(i),i}catch{}return null}function Hx(){return"ontouchstart"in window||navigator.maxTouchPoints>0||navigator.msMaxTouchPoints>0}function zx(){const i=window.innerWidth,t=window.innerHeight,e=Math.min(i,t);return Hx()||e<500?Ii.LOW:e<800?Ii.MEDIUM:Ii.HIGH}const no=()=>gn.gridWidth,io=()=>gn.gridHeight,Ap=()=>gn.gridSize,Ye=()=>gn.worldScale,Ld=()=>gn.maxSimSteps,Gx=()=>no()*Ye(),Wx=()=>io()*Ye(),zn=350,$s=-100,qx=.015,Xx=5,jx=.5,Yx=8,$x=2,Kx=.15,Qx=.01,Zx=5e-5,Nd=10,Ud=6;class Jx{constructor(t){this.container=t,this.terrainWidth=Gx(),this.terrainHeight=Wx(),this.renderer=new Sp({antialias:!0}),this.renderer.setSize(window.innerWidth,window.innerHeight),this.renderer.setPixelRatio(Math.min(window.devicePixelRatio,2)),this.renderer.toneMapping=Xf,this.renderer.toneMappingExposure=1.2,t.appendChild(this.renderer.domElement),this.scene=new Ix,this.scene.background=new oe(8956604),this.setupSkyBackground(),this.scene.fog=new Wl(11193565,8e-5);const e=window.innerWidth/window.innerHeight;this.camera=new Ln(55,e,1,this.terrainWidth*12),this.camera.position.set(this.terrainWidth*.5,zn*2.5,this.terrainHeight*.5),this.controls=new Bx(this.camera,this.renderer.domElement),this.controls.enableDamping=!0,this.controls.dampingFactor=.08,this.controls.rotateSpeed=.8,this.controls.panSpeed=.8,this.controls.zoomSpeed=1.2,this.controls.target.set(0,0,0),this.controls.minPolarAngle=.1,this.controls.maxPolarAngle=Math.PI*.85,this.controls.minDistance=80,this.controls.maxDistance=this.terrainWidth*2.5,this.setupLighting(),this.sunAngle=0,this.sunSpeed=.0375,window.addEventListener("resize",()=>this.onResize())}setupSkyBackground(){const t=new ql(this.terrainWidth*8,32,32),e=new fi({uniforms:{topColor:{value:new oe(6732799)},middleColor:{value:new oe(11193582)},bottomColor:{value:new oe(8956604)},offset:{value:20},exponent:{value:.6}},vertexShader:`
        varying vec3 vWorldPosition;
        void main() {
          vec4 worldPosition = modelMatrix * vec4(position, 1.0);
          vWorldPosition = worldPosition.xyz;
          gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
        }
      `,fragmentShader:`
        uniform vec3 topColor;
        uniform vec3 middleColor;
        uniform vec3 bottomColor;
        uniform float offset;
        uniform float exponent;
        varying vec3 vWorldPosition;
        void main() {
          float h = normalize(vWorldPosition + offset).y;
          if (h > 0.0) {
            gl_FragColor = vec4(mix(middleColor, topColor, pow(h, exponent)), 1.0);
          } else {
            gl_FragColor = vec4(mix(middleColor, bottomColor, pow(-h, 0.5)), 1.0);
          }
        }
      `,side:vn}),n=new Ie(t,e);this.scene.add(n)}setupLighting(){const t=new Nx(8900331,8413248,.8);this.scene.add(t),this.sun=new Ad(16774373,1.6),this.sun.position.set(this.terrainWidth*.4,zn*4,this.terrainHeight*.2),this.scene.add(this.sun);const e=new Ra(this.terrainWidth*.06,32),n=new bn({color:16772676,fog:!1});this.sunDisc=new Ie(e,n),this.sunDisc.position.copy(this.sun.position),this.scene.add(this.sunDisc);const r=new Ra(this.terrainWidth*.1,32),s=new fi({uniforms:{color:{value:new oe(16768341)},opacity:{value:.4}},vertexShader:`
        varying vec2 vUv;
        void main() {
          vUv = uv;
          gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
        }
      `,fragmentShader:`
        uniform vec3 color;
        uniform float opacity;
        varying vec2 vUv;
        void main() {
          float dist = length(vUv - 0.5) * 2.0;
          float alpha = opacity * (1.0 - smoothstep(0.0, 1.0, dist));
          gl_FragColor = vec4(color, alpha);
        }
      `,transparent:!0,depthWrite:!1,fog:!1});this.sunGlow=new Ie(r,s),this.sunGlow.position.copy(this.sun.position),this.scene.add(this.sunGlow);const o=new Ad(11189196,.5);o.position.set(-this.terrainWidth*.3,zn*2,-this.terrainHeight*.4),this.scene.add(o);const a=new Fx(4210768,.3);this.scene.add(a)}onResize(){const t=window.innerWidth,e=window.innerHeight;this.camera.aspect=t/e,this.camera.updateProjectionMatrix(),this.renderer.setSize(t,e)}add(t){this.scene.add(t)}remove(t){this.scene.remove(t)}update(t){this.sunAngle+=this.sunSpeed*t;const e=this.terrainWidth*1.5,n=zn*4+Math.sin(this.sunAngle*.5)*zn*2;this.sun.position.x=Math.cos(this.sunAngle)*e,this.sun.position.z=Math.sin(this.sunAngle)*e,this.sun.position.y=n,this.sun.target.position.set(0,0,0);const r=this.terrainWidth*2.5,s=n*1.5;this.sunDisc.position.x=Math.cos(this.sunAngle)*r,this.sunDisc.position.z=Math.sin(this.sunAngle)*r,this.sunDisc.position.y=s,this.sunGlow.position.copy(this.sunDisc.position),this.sunDisc.lookAt(this.camera.position),this.sunGlow.lookAt(this.camera.position)}render(){this.controls.update(),this.renderer.render(this.scene,this.camera)}get canvas(){return this.renderer.domElement}resetCamera(){this.camera.position.set(this.terrainWidth*.5,zn*2.5,this.terrainHeight*.5),this.controls.target.set(0,0,0),this.controls.update()}}const Ai=new Uint8Array(512),Od=[151,160,137,91,90,15,131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,190,6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,88,237,149,56,87,174,20,125,136,171,168,68,175,74,165,71,134,139,48,27,166,77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,102,143,54,65,25,63,161,1,216,80,73,209,76,132,187,208,89,18,169,200,196,135,130,116,188,159,86,164,100,109,198,173,186,3,64,52,217,226,250,124,123,5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,223,183,170,213,119,248,152,2,44,154,163,70,221,153,101,155,167,43,172,9,129,22,39,253,19,98,108,110,79,113,224,232,178,185,112,104,218,246,97,228,251,34,242,193,238,210,144,12,191,179,162,241,81,51,145,235,249,14,239,107,49,192,214,31,181,199,106,157,184,84,204,176,115,121,50,45,127,4,150,254,138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180];for(let i=0;i<256;i++)Ai[i]=Od[i],Ai[256+i]=Od[i];function Fd(i){return i*i*i*(i*(i*6-15)+10)}function qc(i,t,e){return t+i*(e-t)}function sa(i,t,e){const n=i&3,r=n<2?t:e,s=n<2?e:t;return(n&1?-r:r)+(n&2?-s:s)}function tS(i,t){const e=Math.floor(i)&255,n=Math.floor(t)&255;i-=Math.floor(i),t-=Math.floor(t);const r=Fd(i),s=Fd(t),o=Ai[e]+n,a=Ai[e+1]+n;return qc(s,qc(r,sa(Ai[o],i,t),sa(Ai[a],i-1,t)),qc(r,sa(Ai[o+1],i,t-1),sa(Ai[a+1],i-1,t-1)))}function Vd(i,t,e=4,n=.5){let r=0,s=1,o=1,a=0;for(let c=0;c<e;c++)r+=tS(i*s,t*s)*o,a+=o,o*=n,s*=2;return r/a}const oa={DEFAULT:"default",HILLS:"hills",MOUNTAINS:"mountains",PLAINS:"plains"};class Bd{constructor(){this.width=no(),this.height=io(),this.heightMap=new Float32Array(Ap()),this.dirty=!1,this.currentPreset=oa.DEFAULT,this.brushRadius=Yx,this.brushStrength=$x,this.generate()}index(t,e){return e*this.width+t}inBounds(t,e){return t>=0&&t<this.width&&e>=0&&e<this.height}getHeight(t,e){return this.inBounds(t,e)?this.heightMap[this.index(t,e)]:0}setHeight(t,e,n){this.inBounds(t,e)&&(this.heightMap[this.index(t,e)]=Math.max($s,Math.min(zn,n)),this.dirty=!0)}generate(t=null,e=Math.random()*1e3){t&&(this.currentPreset=t);let n,r,s,o,a,c;switch(this.currentPreset){case oa.HILLS:n=.012,r=4,s=.4,o=.4,a=1.5,c=20;break;case oa.MOUNTAINS:n=.02,r=6,s=.55,o=1,a=3,c=-50;break;case oa.PLAINS:n=.008,r=3,s=.3,o=.15,a=1.2,c=30;break;default:n=qx,r=Xx,s=jx,o=.7,a=2,c=0}for(let h=0;h<this.height;h++)for(let u=0;u<this.width;u++){const d=(u+e)*n,f=(h+e)*n;let p=Vd(d,f,r,s);p+=.3*Vd(d*2+100,f*2+100,2,.5);const y=u/this.width*2-1,E=h/this.height*2-1,_=Math.sqrt(y*y+E*E),m=1-Math.pow(_,a),C=c+p*Math.max(0,m)*zn*o;this.heightMap[this.index(u,h)]=Math.max($s,Math.min(zn,C))}this.dirty=!0}applyBrush(t,e,n=null){const r=this.brushRadius,s=n??this.brushStrength;for(let o=-r;o<=r;o++)for(let a=-r;a<=r;a++){const c=Math.floor(t+a),h=Math.floor(e+o);if(!this.inBounds(c,h))continue;const u=Math.sqrt(a*a+o*o);if(u>r)continue;const d=Math.exp(-(u*u)/(r*r/2)),f=s*d,p=this.index(c,h);this.heightMap[p]=Math.max($s,Math.min(zn,this.heightMap[p]+f))}this.dirty=!0}raise(t,e){this.applyBrush(t,e,Math.abs(this.brushStrength))}lower(t,e){this.applyBrush(t,e,-Math.abs(this.brushStrength))}reset(){this.generate()}getHeightInterpolated(t,e){const n=Math.floor(t),r=Math.floor(e),s=n+1,o=r+1,a=t-n,c=e-r,h=this.getHeight(n,r),u=this.getHeight(s,r),d=this.getHeight(n,o),f=this.getHeight(s,o),p=h*(1-a)+u*a,y=d*(1-a)+f*a;return p*(1-c)+y*c}getSnapshot(){return{width:this.width,height:this.height,heightMap:new Float32Array(this.heightMap),preset:this.currentPreset,brushRadius:this.brushRadius,brushStrength:this.brushStrength}}loadFromSnapshot(t){const e=t.width,n=t.height,r=t.heightMap;this.currentPreset=t.preset,this.brushRadius=t.brushRadius,this.brushStrength=t.brushStrength;for(let s=0;s<this.height;s++)for(let o=0;o<this.width;o++){const a=o/(this.width-1)*(e-1),c=s/(this.height-1)*(n-1),h=Math.floor(a),u=Math.floor(c),d=Math.min(h+1,e-1),f=Math.min(u+1,n-1),p=a-h,y=c-u,E=r[u*e+h],_=r[u*e+d],m=r[f*e+h],C=r[f*e+d],w=E*(1-p)+_*p,b=m*(1-p)+C*p,N=w*(1-y)+b*y;this.heightMap[this.index(o,s)]=N}this.dirty=!0}}const Xc=[[-1,0,1],[1,0,1],[0,-1,1],[0,1,1],[-1,-1,1.414],[1,-1,1.414],[-1,1,1.414],[1,1,1.414]];class kd{constructor(t){this.terrain=t,this.width=no(),this.height=io();const e=Ap();this.depth=new Float32Array(e),this.depthNext=new Float32Array(e),this.velocityX=new Float32Array(e),this.velocityY=new Float32Array(e),this.previousDepth=new Float32Array(e),this.previousTerrain=new Float32Array(e),this.waveEnergy=new Float32Array(e),this.waveDirectionX=new Float32Array(e),this.waveDirectionY=new Float32Array(e),this.waveEnergyNext=new Float32Array(e),this.springs=[],this.flowRate=Kx,this.evaporationRate=Zx,this.minDepth=Qx,this.waveDecay=.96,this.wavePropagation=.35,this.displacementSensitivity=15,this.impactWaveStrength=2,this.pressureWaveStrength=.8,this.activeRegion={minX:0,maxX:this.width-1,minY:0,maxY:this.height-1,needsRecalc:!0},this.regionPadding=3,this.regionRecalcInterval=30,this.framesSinceRecalc=0,this.dirty=!1}recalcActiveRegion(){let t=this.width,e=0,n=this.height,r=0,s=!1;for(const o of this.springs)this.inBounds(o.x,o.y)&&(t=Math.min(t,o.x),e=Math.max(e,o.x),n=Math.min(n,o.y),r=Math.max(r,o.y),s=!0);for(let o=0;o<this.height;o++)for(let a=0;a<this.width;a++){const c=this.index(a,o);(this.depth[c]>this.minDepth||this.waveEnergy[c]>.05)&&(t=Math.min(t,a),e=Math.max(e,a),n=Math.min(n,o),r=Math.max(r,o),s=!0)}s?(this.activeRegion.minX=Math.max(1,t-this.regionPadding),this.activeRegion.maxX=Math.min(this.width-2,e+this.regionPadding),this.activeRegion.minY=Math.max(1,n-this.regionPadding),this.activeRegion.maxY=Math.min(this.height-2,r+this.regionPadding)):(this.activeRegion.minX=1,this.activeRegion.maxX=1,this.activeRegion.minY=1,this.activeRegion.maxY=1),this.activeRegion.needsRecalc=!1,this.framesSinceRecalc=0}expandRegion(t,e,n=0){const r=this.regionPadding+n;this.activeRegion.minX=Math.max(1,Math.min(this.activeRegion.minX,t-r)),this.activeRegion.maxX=Math.min(this.width-2,Math.max(this.activeRegion.maxX,t+r)),this.activeRegion.minY=Math.max(1,Math.min(this.activeRegion.minY,e-r)),this.activeRegion.maxY=Math.min(this.height-2,Math.max(this.activeRegion.maxY,e+r))}index(t,e){return e*this.width+t}inBounds(t,e){return t>=0&&t<this.width&&e>=0&&e<this.height}getDepth(t,e){return this.inBounds(t,e)?this.depth[this.index(t,e)]:0}addWater(t,e,n=10,r=3){const s=Math.PI*r*r,o=n/Math.max(1,s*.5);for(let a=-r;a<=r;a++)for(let c=-r;c<=r;c++){const h=Math.floor(t+c),u=Math.floor(e+a);if(!this.inBounds(h,u))continue;const d=Math.sqrt(c*c+a*a);if(d>r)continue;const f=1-d/r,p=this.index(h,u);this.depth[p]+=o*f}this.expandRegion(Math.floor(t),Math.floor(e),r),this.dirty=!0}addSpring(t,e,n=Nd){const r=Math.floor(t),s=Math.floor(e);this.springs.push({x:r,y:s,flowRate:n}),this.expandRegion(r,s,5)}removeSpringNear(t,e,n=3){this.springs=this.springs.filter(r=>{const s=r.x-t,o=r.y-e;return Math.sqrt(s*s+o*o)>n})}clearSprings(){this.springs=[]}reset(){this.depth.fill(0),this.depthNext.fill(0),this.velocityX.fill(0),this.velocityY.fill(0),this.previousDepth.fill(0),this.previousTerrain.set(this.terrain.heightMap),this.waveEnergy.fill(0),this.waveEnergyNext.fill(0),this.waveDirectionX.fill(0),this.waveDirectionY.fill(0),this.activeRegion.needsRecalc=!0,this.dirty=!0}simulate(t=1/60){const e=this.terrain.heightMap;this.framesSinceRecalc++,(this.activeRegion.needsRecalc||this.framesSinceRecalc>=this.regionRecalcInterval)&&this.recalcActiveRegion();const{minX:n,maxX:r,minY:s,maxY:o}=this.activeRegion;this.depthNext.set(this.depth),this.waveEnergyNext.set(this.waveEnergy);for(const c of this.springs)if(this.inBounds(c.x,c.y)){const h=this.index(c.x,c.y);this.depthNext[h]+=c.flowRate*t;const u=c.flowRate*.5*t;this.waveEnergyNext[h]+=u;for(const[d,f]of[[-1,0],[1,0],[0,-1],[0,1]]){const p=c.x+d,y=c.y+f;if(this.inBounds(p,y)){const E=this.index(p,y);this.waveEnergyNext[E]+=u*.3,this.waveDirectionX[E]+=d*u,this.waveDirectionY[E]+=f*u}}}for(let c=s;c<=o;c++)for(let h=n;h<=r;h++){const u=this.index(h,c);if(this.depth[u]>this.minDepth){const f=e[u]-this.previousTerrain[u];if(Math.abs(f)>.1){const p=Math.abs(f)*this.displacementSensitivity;this.waveEnergyNext[u]+=p;const y=(e[this.index(h+1,c)]-e[this.index(h-1,c)])*.5,E=(e[this.index(h,c+1)]-e[this.index(h,c-1)])*.5,_=Math.sqrt(y*y+E*E)+.001;this.waveDirectionX[u]+=(f>0?-y:y)/_*p,this.waveDirectionY[u]+=(f>0?-E:E)/_*p}}}for(let c=s;c<=o;c++)for(let h=n;h<=r;h++){const u=this.index(h,c),d=this.depth[u];if(d<this.minDepth)continue;const f=e[u],p=f+d;let y=0,E=0,_=0,m=0,C=0,w=0;for(const[N,U,T]of Xc){const M=h+N,v=c+U,S=this.index(M,v),P=e[S],L=this.depth[S],R=P+L;N!==0&&(C+=(R-p)*N/T),U!==0&&(w+=(R-p)*U/T);const V=p-R;if(V>0){if(P-f>d*.5){m+=V*this.flowRate/T;continue}const K=d*.25,Z=Math.min(V*this.flowRate/T,K);Z>1e-4&&(this.depthNext[u]-=Z,this.depthNext[S]+=Z,y+=Z,E+=N*Z,_+=U*Z)}}if(m>.05){const N=m*this.impactWaveStrength;this.waveEnergyNext[u]+=N,this.waveDirectionX[u]-=this.velocityX[u]*N*.5,this.waveDirectionY[u]-=this.velocityY[u]*N*.5}const b=Math.sqrt(C*C+w*w);if(b>.1){const N=b*this.pressureWaveStrength*d*.1;this.waveEnergyNext[u]+=N,this.waveDirectionX[u]+=C*N,this.waveDirectionY[u]+=w*N}y>0?(this.velocityX[u]=this.velocityX[u]*.8+E*.2,this.velocityY[u]=this.velocityY[u]*.8+_*.2):(this.velocityX[u]*=.95,this.velocityY[u]*=.95)}if(n>1){for(let c=s;c<=o;c++)if(this.depthNext[this.index(n,c)]>this.minDepth){this.activeRegion.minX=Math.max(1,n-2);break}}if(r<this.width-2){for(let c=s;c<=o;c++)if(this.depthNext[this.index(r,c)]>this.minDepth){this.activeRegion.maxX=Math.min(this.width-2,r+2);break}}if(s>1){for(let c=n;c<=r;c++)if(this.depthNext[this.index(c,s)]>this.minDepth){this.activeRegion.minY=Math.max(1,s-2);break}}if(o<this.height-2){for(let c=n;c<=r;c++)if(this.depthNext[this.index(c,o)]>this.minDepth){this.activeRegion.maxY=Math.min(this.height-2,o+2);break}}for(let c=s;c<=o;c++)for(let h=n;h<=r;h++){const u=this.index(h,c),d=this.waveEnergy[u];if(d>.05&&this.depth[u]>this.minDepth){const f=this.waveDirectionX[u],p=this.waveDirectionY[u],y=Math.sqrt(f*f+p*p)+.001,E=f/y,_=p/y;for(const[m,C,w]of Xc){const b=h+m,N=c+C,U=this.index(b,N);if(this.depth[U]>this.minDepth){const T=(m*E+C*_)/w,M=Math.max(.1,.5+T*.5),v=d*this.wavePropagation*M/8;this.waveEnergyNext[U]+=v,this.waveEnergyNext[u]-=v,this.waveDirectionX[U]+=E*v*.5,this.waveDirectionY[U]+=_*v*.5}}}}for(let c=s;c<=o;c++)for(let h=n;h<=r;h++){const u=this.index(h,c);this.waveEnergyNext[u]*=this.waveDecay,this.waveDirectionX[u]*=.92,this.waveDirectionY[u]*=.92}for(let c=s;c<=o;c++)for(let h=n;h<=r;h++){const u=this.index(h,c);this.depthNext[u]>0&&(this.depthNext[u]=Math.max(0,this.depthNext[u]-this.evaporationRate*t))}this.previousDepth.set(this.depth),this.previousTerrain.set(e);let a=this.depth;this.depth=this.depthNext,this.depthNext=a,a=this.waveEnergy,this.waveEnergy=this.waveEnergyNext,this.waveEnergyNext=a,this.dirty=!0}getTotalWater(){let t=0;for(let e=0;e<this.width*this.height;e++)t+=this.depth[e];return t}findPeaks(t=Ud){const e=this.terrain.heightMap,n=[],r=10;for(let a=r;a<this.height-r;a++)for(let c=r;c<this.width-r;c++){const h=this.index(c,a),u=e[h];let d=!0;for(const[f,p]of Xc){const y=this.index(c+f,a+p);if(e[y]>=u){d=!1;break}}d&&u>20&&n.push({x:c,y:a,height:u})}n.sort((a,c)=>c.height-a.height);const s=[],o=15;for(const a of n){let c=!1;for(const h of s){const u=a.x-h.x,d=a.y-h.y;if(Math.sqrt(u*u+d*d)<o){c=!0;break}}if(!c&&(s.push(a),s.length>=t))break}return s}autoGenerateSprings(t=Ud){this.clearSprings();const e=this.findPeaks(t);for(const n of e)this.addSpring(n.x,n.y,Nd);console.log(`Generated ${this.springs.length} springs at peaks`)}getSnapshot(){return{width:this.width,height:this.height,depth:new Float32Array(this.depth),waveEnergy:new Float32Array(this.waveEnergy),springs:this.springs.map(t=>({...t}))}}loadFromSnapshot(t){const e=t.width,n=t.height,r=t.depth,s=t.waveEnergy;for(let o=0;o<this.height;o++)for(let a=0;a<this.width;a++){const c=a/(this.width-1)*(e-1),h=o/(this.height-1)*(n-1),u=Math.floor(c),d=Math.floor(h),f=Math.min(u+1,e-1),p=Math.min(d+1,n-1),y=c-u,E=h-d,_=r[d*e+u],m=r[d*e+f],C=r[p*e+u],w=r[p*e+f],b=_*(1-y)+m*y,N=C*(1-y)+w*y,U=b*(1-E)+N*E,T=this.index(a,o);this.depth[T]=U,this.depthNext[T]=U;const M=s[d*e+u],v=s[d*e+f],S=s[p*e+u],P=s[p*e+f],L=M*(1-y)+v*y,R=S*(1-y)+P*y;this.waveEnergy[T]=L*(1-E)+R*E}this.springs=t.springs.map(o=>({x:Math.floor(o.x/(e-1)*(this.width-1)),y:Math.floor(o.y/(n-1)*(this.height-1)),flowRate:o.flowRate})),this.dirty=!0}}class eS{constructor(t){this.terrain=t;const e=Ye();this.geometry=new dr(t.width*e,t.height*e,t.width-1,t.height-1),this.geometry.rotateX(-Math.PI/2),this.material=new wp({vertexColors:!0,flatShading:!1,roughness:.8,metalness:.1}),this.mesh=new Ie(this.geometry,this.material),this.mesh.receiveShadow=!0,this.updateGeometry()}updateGeometry(){var s;const t=this.geometry.attributes.position.array,e=((s=this.geometry.attributes.color)==null?void 0:s.array)||new Float32Array(t.length),n=this.terrain.width,r=this.terrain.height;for(let o=0;o<r;o++)for(let a=0;a<n;a++){const c=o*n+a,h=c*3,u=this.terrain.heightMap[c];t[h+1]=u;const d=this.getHeightColor(u);e[h]=d.r,e[h+1]=d.g,e[h+2]=d.b}this.geometry.attributes.color||this.geometry.setAttribute("color",new xn(e,3)),this.geometry.attributes.position.needsUpdate=!0,this.geometry.attributes.color.needsUpdate=!0,this.geometry.computeVertexNormals(),this.terrain.dirty=!1}getHeightColor(t){const e=(t-$s)/(zn-$s);return e<.1?this.lerpColor({r:.12,g:.11,b:.1},{r:.22,g:.21,b:.19},e/.1):e<.15?this.lerpColor({r:.22,g:.21,b:.19},{r:.12,g:.22,b:.12},(e-.1)/.05):e<.3?this.lerpColor({r:.12,g:.22,b:.12},{r:.15,g:.35,b:.25},(e-.15)/.15):e<.35?this.lerpColor({r:.82,g:.72,b:.45},{r:.65,g:.55,b:.35},(e-.3)/.05):e<.6?this.lerpColor({r:.18,g:.55,b:.15},{r:.25,g:.48,b:.18},(e-.35)/.25):e<.75?this.lerpColor({r:.1,g:.38,b:.1},{r:.22,g:.32,b:.18},(e-.6)/.15):e<.9?this.lerpColor({r:.35,g:.34,b:.32},{r:.52,g:.5,b:.48},(e-.75)/.15):this.lerpColor({r:.85,g:.85,b:.9},{r:.98,g:.98,b:1},(e-.9)/.1)}lerpColor(t,e,n){return n=Math.max(0,Math.min(1,n)),{r:t.r+(e.r-t.r)*n,g:t.g+(e.g-t.g)*n,b:t.b+(e.b-t.b)*n}}update(){this.terrain.dirty&&this.updateGeometry()}rebuild(){const t=Ye();this.geometry.dispose(),this.geometry=new dr(this.terrain.width*t,this.terrain.height*t,this.terrain.width-1,this.terrain.height-1),this.geometry.rotateX(-Math.PI/2),this.mesh.geometry=this.geometry,this.updateGeometry()}get object(){return this.mesh}}const aa=.3,jc=.8,Yc=.06,Hd=1.5,nS=4,iS=.5,rS=1.5;class sS{constructor(t,e){this.water=t,this.terrain=e,this.time=0,this.gridWidth=t.width,this.gridHeight=t.height,this.worldScale=Ye(),this.geometry=new dr(this.gridWidth*this.worldScale,this.gridHeight*this.worldScale,this.gridWidth-1,this.gridHeight-1),this.geometry.rotateX(-Math.PI/2);const n=this.gridWidth*this.gridHeight;this.baseHeights=new Float32Array(n),this.smoothedWaveHeights=new Float32Array(n),this.waveSmoothing=.15;const r=new Float32Array(n*4);this.geometry.setAttribute("color",new xn(r,4)),this.material=new wp({color:3381759,transparent:!0,opacity:1,roughness:.15,metalness:.6,side:ui,vertexColors:!0,depthWrite:!1,envMapIntensity:1.5,emissive:1140394,emissiveIntensity:.15}),this.material.onBeforeCompile=s=>{s.fragmentShader=s.fragmentShader.replace("#include <color_fragment>",`#include <color_fragment>
         diffuseColor.a *= vColor.a;`)},this.mesh=new Ie(this.geometry,this.material),this.mesh.renderOrder=1,this.updateGeometry()}isShorelineCell(t,e,n){if(n[e*this.gridWidth+t]<=aa)return!1;const s=[[-1,0],[1,0],[0,-1],[0,1]];for(const[o,a]of s){const c=t+o,h=e+a;if(c<0||c>=this.gridWidth||h<0||h>=this.gridHeight)continue;if(n[h*this.gridWidth+c]<=aa)return!0}return!1}getFlowSpeed(t,e){const n=e*this.gridWidth+t,r=this.water.velocityX[n],s=this.water.velocityY[n];return Math.sqrt(r*r+s*s)}getWaveEnergy(t,e){const n=e*this.gridWidth+t;return this.water.waveEnergy[n]}updateGeometry(){this.geometry.attributes.position.array;const t=this.geometry.attributes.color.array,e=this.terrain.heightMap,n=this.water.depth,{minX:r,maxX:s,minY:o,maxY:a}=this.water.activeRegion,c=2;let h=!1;const u={r:.3,g:.75,b:.8},d={r:.15,g:.45,b:.75},f={r:.05,g:.15,b:.4},p={r:.9,g:.95,b:1};for(let y=0;y<this.gridHeight;y++)for(let E=0;E<this.gridWidth;E++){const _=y*this.gridWidth+E,m=_*4,C=E>=r-c&&E<=s+c&&y>=o-c&&y<=a+c,w=n[_],b=e[_];if(!C){this.baseHeights[_]=b-10,t[m]=0,t[m+1]=0,t[m+2]=0,t[m+3]=0;continue}if(w>aa){this.baseHeights[_]=b+w;const N=2,U=6,T=15;let M,v,S;if(w<N)M=u.r,v=u.g,S=u.b;else if(w<U){const tt=(w-N)/(U-N);M=u.r+(d.r-u.r)*tt,v=u.g+(d.g-u.g)*tt,S=u.b+(d.b-u.b)*tt}else if(w<T){const tt=(w-U)/(T-U);M=d.r+(f.r-d.r)*tt,v=d.g+(f.g-d.g)*tt,S=d.b+(f.b-d.b)*tt}else M=f.r,v=f.g,S=f.b;const P=this.isShorelineCell(E,y,n),L=this.getFlowSpeed(E,y),R=this.getWaveEnergy(E,y);let V=0;P&&(V+=.5),V+=Math.min(.4,L*.3),V+=Math.min(.3,R*.15),w<N&&(V+=.1*(1-w/N)),V=Math.min(.8,V);const G=M+(p.r-M)*V,K=v+(p.g-v)*V,Z=S+(p.b-S)*V,J=Math.min(1,.5+w/10),et=Math.min(.9,J*.85+V*.1);t[m]=G,t[m+1]=K,t[m+2]=Z,t[m+3]=et,h=!0}else this.baseHeights[_]=b-10,t[m]=0,t[m+1]=0,t[m+2]=0,t[m+3]=0}this.geometry.attributes.color.needsUpdate=!0,this.mesh.visible=h,this.water.dirty=!1,this.updateWaveAnimation(0)}updateWaveAnimation(t){this.time+=t;const e=this.geometry.attributes.position.array,n=this.geometry.attributes.color.array,r=this.water.depth,s=this.water.waveEnergy,o=this.water.waveDirectionX,a=this.water.waveDirectionY,{minX:c,maxX:h,minY:u,maxY:d}=this.water.activeRegion,f=2;for(let p=0;p<this.gridHeight;p++)for(let y=0;y<this.gridWidth;y++){const E=p*this.gridWidth+y,_=E*3,m=E*4;if(!(y>=c-f&&y<=h+f&&p>=u-f&&p<=d+f)){e[_+1]=this.baseHeights[E];continue}const w=r[E];if(w>aa){const b=y*this.worldScale,N=p*this.worldScale,U=this.water.velocityX[E],T=this.water.velocityY[E],M=Math.sqrt(U*U+T*T),v=M>.01?U/M:0,S=M>.01?T/M:0,P=-S,L=v,R=Math.min(1,w/3),V=Math.min(1,M*.5),G=s[E],K=Math.min(1,M*3),Z=Math.min(1,G*.8),et=Math.max(K,Z),tt=Math.sin(b*Yc+this.time*jc*.5)*Math.cos(N*Yc*.8+this.time*jc*.4),pt=Math.sin((b+N)*Yc*.6+this.time*jc*.3)*.5,ht=(tt+pt)*Hd*(1-V)*R*et;let X=0;if(M>.1){const Ft=b*v+N*S,wt=b*P+N*L,Wt=Math.sin(wt*.12+Ft*.03-this.time*M*.8),W=Math.sin(wt*.08-this.time*M*.5)*.6,pe=M>.5?Math.sin(Ft*.2+this.time*3)*Math.sin(wt*.15+this.time*2)*.3:0;X=(Wt+W+pe)*Hd*1.5*V*R}let it=0;if(G>.05){const Ft=o[E],wt=a[E],Wt=Math.sqrt(Ft*Ft+wt*wt)+.001,W=Ft/Wt,pe=wt/Wt,Rt=(b*W+N*pe)*.1,kt=Math.min(1,Math.sqrt(G*iS));it=Math.sin(Rt+this.time*rS)*nS*kt*R}let mt=0;if(w>.5){const Ft=M>.1?M*2:.5,wt=M>.1?b*v+N*S:b+N,Wt=Math.sin(wt*.15+this.time*Ft),W=Math.sin(b*.12-N*.08+this.time*.7);mt=(Wt*W+1)*.5;const pe=.15*Math.min(1,w/5),Rt=n[m],kt=n[m+1],bt=n[m+2];n[m]=Rt+mt*pe,n[m+1]=kt+mt*pe*1.1,n[m+2]=bt+mt*pe*.5}const Pt=ht+X+it,Mt=this.smoothedWaveHeights[E],Gt=Mt+(Pt-Mt)*this.waveSmoothing;this.smoothedWaveHeights[E]=Gt,e[_+1]=this.baseHeights[E]+Gt}else this.smoothedWaveHeights[E]=0,e[_+1]=this.baseHeights[E]}this.geometry.attributes.position.needsUpdate=!0,this.geometry.attributes.color.needsUpdate=!0,this.geometry.computeVertexNormals()}update(t=1/60){this.water.dirty||this.terrain.dirty?this.updateGeometry():this.updateWaveAnimation(t)}rebuild(){this.gridWidth=this.water.width,this.gridHeight=this.water.height,this.worldScale=Ye(),this.geometry.dispose(),this.geometry=new dr(this.gridWidth*this.worldScale,this.gridHeight*this.worldScale,this.gridWidth-1,this.gridHeight-1),this.geometry.rotateX(-Math.PI/2);const t=this.gridWidth*this.gridHeight;this.baseHeights=new Float32Array(t),this.smoothedWaveHeights=new Float32Array(t);const e=new Float32Array(t*4);this.geometry.setAttribute("color",new xn(e,4)),this.mesh.geometry=this.geometry,this.updateGeometry()}get object(){return this.mesh}}class oS{constructor(t,e){this.water=t,this.terrain=e,this.worldScale=Ye(),this.group=new rr,this.ringGeometry=new mo(3,5,16),this.ringMaterial=new bn({color:4500172,transparent:!0,opacity:.4,side:_n}),this.markerPool=[],this.lastSpringCount=0}update(t=1/60){const e=this.water.springs;e.length!==this.lastSpringCount&&(this.rebuild(),this.lastSpringCount=e.length);for(let n=0;n<e.length;n++){const r=e[n],s=this.markerPool[n];if(!s)continue;const o=this.terrain.getHeight(r.x,r.y),a=this.water.getDepth(r.x,r.y),c=(r.x-this.terrain.width/2)*this.worldScale,h=(r.y-this.terrain.height/2)*this.worldScale,u=o+Math.max(.5,a)+.2;s.position.set(c,u,h)}}rebuild(){const t=this.water.springs;for(let e=t.length;e<this.markerPool.length;e++)this.markerPool[e].visible=!1;for(let e=0;e<t.length;e++){let n=this.markerPool[e];n||(n=new Ie(this.ringGeometry,this.ringMaterial),n.rotation.x=-Math.PI/2,this.markerPool.push(n),this.group.add(n)),n.visible=!0}}refreshScale(){this.worldScale=Ye(),this.lastSpringCount=-1}get object(){return this.group}}const Cn={RAISE:"raise",LOWER:"lower",WATER:"water",SPRING:"spring",REMOVE_SPRING:"remove spring"};class aS{constructor(t,e,n){Vn(this,"onAddWater",null);Vn(this,"onAddSpring",null);Vn(this,"onRemoveSpring",null);Vn(this,"onClearWater",null);Vn(this,"onTimeScaleChange",null);Vn(this,"onTogglePause",null);Vn(this,"onResetCamera",null);this.scene=t,this.terrain=e,this.terrainMesh=n,this.currentTool=Cn.RAISE,this.isEditing=!1,this.raycaster=new bp,this.mouse=new Vt,this.fps=0,this.waterVolume=0,this.springCount=0,this.brushCursor=this.createBrushCursor(),this.scene.add(this.brushCursor),this.orbitControls=this.scene.controls,this.orbitControls.enabled=!0,this.orbitControls.mouseButtons={LEFT:null,MIDDLE:wi.DOLLY,RIGHT:wi.ROTATE},this.orbitControls.enablePan=!0,this.orbitControls.enableZoom=!0,this.orbitControls.enableDamping=!0,this.orbitControls.screenSpacePanning=!0,this.keys={w:!1,a:!1,s:!1,d:!1,q:!1,e:!1},this.panSpeed=400,this.onMouseDown=this.onMouseDown.bind(this),this.onMouseUp=this.onMouseUp.bind(this),this.onMouseMove=this.onMouseMove.bind(this),this.onContextMenu=this.onContextMenu.bind(this),this.onKeyDown=this.onKeyDown.bind(this),this.onKeyUp=this.onKeyUp.bind(this),this.setupEventListeners()}createBrushCursor(){const t=new mo(.9,1,32);t.rotateX(-Math.PI/2);const e=new bn({color:16777215,transparent:!0,opacity:.6,side:_n,depthTest:!1}),n=new Ie(t,e);return n.renderOrder=999,n.visible=!1,n}updateBrushCursor(t,e,n){const r=this.terrain.brushRadius*Ye();this.brushCursor.scale.set(r,r,r),this.brushCursor.position.set(t,e+1,n),this.brushCursor.visible=!0}hideBrushCursor(){this.brushCursor.visible=!1}setupEventListeners(){const t=this.scene.canvas;t.addEventListener("mousedown",this.onMouseDown),t.addEventListener("mouseup",this.onMouseUp),t.addEventListener("mousemove",this.onMouseMove),t.addEventListener("mouseleave",()=>this.hideBrushCursor()),t.addEventListener("contextmenu",this.onContextMenu),window.addEventListener("keydown",this.onKeyDown),window.addEventListener("keyup",this.onKeyUp)}onContextMenu(t){t.preventDefault()}onKeyDown(t){const e=t.key.toLowerCase();if(e in this.keys){this.keys[e]=!0;return}switch(e){case"1":this.currentTool=Cn.RAISE,this.updateToolDisplay();break;case"2":this.currentTool=Cn.LOWER,this.updateToolDisplay();break;case"3":this.currentTool=Cn.WATER,this.updateToolDisplay();break;case"4":this.currentTool=Cn.SPRING,this.updateToolDisplay();break;case"5":this.currentTool=Cn.REMOVE_SPRING,this.updateToolDisplay();break;case"r":this.terrain.reset();break;case"c":this.onClearWater&&this.onClearWater();break;case"[":this.terrain.brushRadius=Math.max(1,this.terrain.brushRadius-1),this.updateToolDisplay();break;case"]":this.terrain.brushRadius=Math.min(30,this.terrain.brushRadius+1),this.updateToolDisplay();break;case"-":this.terrain.brushStrength=Math.max(.5,this.terrain.brushStrength-.5),this.updateToolDisplay();break;case"=":case"+":this.terrain.brushStrength=Math.min(20,this.terrain.brushStrength+.5),this.updateToolDisplay();break;case",":case"<":this.onTimeScaleChange&&this.onTimeScaleChange(-.25);break;case".":case">":this.onTimeScaleChange&&this.onTimeScaleChange(.25);break;case"p":this.onTogglePause&&this.onTogglePause();break;case"h":this.onResetCamera&&this.onResetCamera();break}}onKeyUp(t){const e=t.key.toLowerCase();e in this.keys&&(this.keys[e]=!1)}updateToolDisplay(){const t=document.getElementById("controls-panel");if(t){const e=t.querySelector(".tool-name"),n=t.querySelector(".brush-info");e&&(e.textContent=this.currentTool),n&&(n.textContent=`Size: ${this.terrain.brushRadius} | Strength: ${this.terrain.brushStrength.toFixed(1)}`)}}updateStats(t,e,n,r=1,s=!1){this.fps=Math.round(t),this.waterVolume=e,this.springCount=n;const o=document.getElementById("controls-panel");if(o){const a=o.querySelector(".stats");if(a){let c=s?"PAUSED":`${r.toFixed(2)}x`;a.textContent=`${this.fps} FPS | ${c} | Water: ${this.waterVolume.toFixed(0)} | Springs: ${this.springCount}`}}}onMouseDown(t){t.button===0&&(this.isEditing=!0,this.applyToolAtMouse(t))}onMouseUp(t){t.button===0&&(this.isEditing=!1)}onMouseMove(t){const e=this.scene.canvas.getBoundingClientRect();this.mouse.x=(t.clientX-e.left)/e.width*2-1,this.mouse.y=-((t.clientY-e.top)/e.height)*2+1,this.raycaster.setFromCamera(this.mouse,this.scene.camera);const n=this.raycaster.intersectObject(this.terrainMesh.mesh);if(n.length>0){const r=n[0].point;this.updateBrushCursor(r.x,r.y,r.z)}else this.hideBrushCursor();this.isEditing&&t.buttons&1&&this.currentTool!==Cn.SPRING&&this.applyToolAtMouse(t)}updateCamera(t){const e=this.scene.camera,n=this.orbitControls.target,r=new k;e.getWorldDirection(r),r.y=0,r.normalize();const s=new k;s.crossVectors(r,new k(0,1,0)).normalize();const o=this.panSpeed*t;this.keys.w&&(e.position.add(r.clone().multiplyScalar(o)),n.add(r.clone().multiplyScalar(o))),this.keys.s&&(e.position.add(r.clone().multiplyScalar(-o)),n.add(r.clone().multiplyScalar(-o))),this.keys.d&&(e.position.add(s.clone().multiplyScalar(o)),n.add(s.clone().multiplyScalar(o))),this.keys.a&&(e.position.add(s.clone().multiplyScalar(-o)),n.add(s.clone().multiplyScalar(-o))),this.keys.e&&(e.position.y+=o,n.y+=o),this.keys.q&&(e.position.y-=o,n.y-=o)}applyToolAtMouse(t){const e=this.scene.canvas.getBoundingClientRect();this.mouse.x=(t.clientX-e.left)/e.width*2-1,this.mouse.y=-((t.clientY-e.top)/e.height)*2+1,this.raycaster.setFromCamera(this.mouse,this.scene.camera);const n=this.raycaster.intersectObject(this.terrainMesh.mesh);if(n.length>0){const r=n[0].point,s=Ye(),o=r.x/s+this.terrain.width/2,a=r.z/s+this.terrain.height/2;switch(this.currentTool){case Cn.RAISE:this.terrain.raise(o,a);break;case Cn.LOWER:this.terrain.lower(o,a);break;case Cn.WATER:this.onAddWater&&this.onAddWater(o,a);break;case Cn.SPRING:this.onAddSpring&&this.onAddSpring(o,a);break;case Cn.REMOVE_SPRING:this.onRemoveSpring&&this.onRemoveSpring(o,a);break}}}}const jr={RAISE:"raise",LOWER:"lower",WATER:"water",SPRING:"spring"};class cS{constructor(t,e,n){Vn(this,"onAddWater",null);Vn(this,"onAddSpring",null);Vn(this,"onTerrainReset",null);this.scene=t,this.terrain=e,this.terrainMesh=n,this.worldScale=Ye(),this.gridWidth=no(),this.gridHeight=io(),this.currentTool=jr.RAISE,this.raycaster=new bp,this.touchPoint=new Vt,this.activeTouches=new Map,this.lastTouchDist=0,this.lastTouchCenter={x:0,y:0},this.lastTouchAngle=0,this.isSingleTouch=!1,this.toolAppliedThisGesture=!1,this.brushCursor=this.createBrushCursor(),this.scene.add(this.brushCursor),this.camera=this.scene.camera,this.orbitTarget=this.scene.controls.target,this.scene.controls.enabled=!1,this.onTouchStart=this.onTouchStart.bind(this),this.onTouchMove=this.onTouchMove.bind(this),this.onTouchEnd=this.onTouchEnd.bind(this),this.setupTouchListeners(),this.setupUIListeners()}refreshScale(){this.worldScale=Ye(),this.gridWidth=no(),this.gridHeight=io()}createBrushCursor(){const t=new mo(.9,1,32);t.rotateX(-Math.PI/2);const e=new bn({color:16777215,transparent:!0,opacity:.6,side:_n,depthTest:!1}),n=new Ie(t,e);return n.renderOrder=999,n.visible=!1,n}updateBrushCursor(t,e,n){const r=this.terrain.brushRadius*this.worldScale;this.brushCursor.scale.set(r,r,r),this.brushCursor.position.set(t,e+1,n),this.brushCursor.visible=!0}hideBrushCursor(){this.brushCursor.visible=!1}setupTouchListeners(){const t=this.scene.canvas;t.addEventListener("touchstart",this.onTouchStart,{passive:!1}),t.addEventListener("touchmove",this.onTouchMove,{passive:!1}),t.addEventListener("touchend",this.onTouchEnd,{passive:!1}),t.addEventListener("touchcancel",this.onTouchEnd,{passive:!1})}setupUIListeners(){const t=document.querySelectorAll(".tool-btn");t.forEach(b=>{b.addEventListener("touchstart",N=>{N.stopPropagation();const U=b.dataset.tool;this.currentTool=U,t.forEach(T=>T.classList.remove("active")),b.classList.add("active")}),b.addEventListener("click",N=>{N.stopPropagation();const U=b.dataset.tool;this.currentTool=U,t.forEach(T=>T.classList.remove("active")),b.classList.add("active")})});const e=document.getElementById("btn-new-map"),n=document.getElementById("preset-dropdown");if(e&&n){const b=N=>{var U;N.preventDefault(),N.stopPropagation(),n.classList.toggle("visible"),(U=document.getElementById("quality-dropdown"))==null||U.classList.remove("visible")};e.addEventListener("touchend",b),e.addEventListener("click",N=>{var U;(U=N.sourceCapabilities)!=null&&U.firesTouchEvents||b(N)}),document.addEventListener("touchend",N=>{setTimeout(()=>{n.classList.contains("visible")&&!n.contains(N.target)&&N.target!==e&&n.classList.remove("visible")},50)})}const r=document.querySelectorAll(".preset-btn");r.forEach(b=>{const N=U=>{U.preventDefault(),U.stopPropagation();const T=b.dataset.preset;this.terrain.generate(T),this.onTerrainReset&&this.onTerrainReset(),r.forEach(M=>M.classList.remove("active")),b.classList.add("active"),n&&n.classList.remove("visible")};b.addEventListener("touchend",N),b.addEventListener("click",U=>{var T;(T=U.sourceCapabilities)!=null&&T.firesTouchEvents||N(U)})});const s=document.getElementById("btn-reset");if(s){const b=N=>{N.preventDefault(),N.stopPropagation(),this.terrain.reset(),this.onTerrainReset&&this.onTerrainReset()};s.addEventListener("touchend",b),s.addEventListener("click",N=>{var U;(U=N.sourceCapabilities)!=null&&U.firesTouchEvents||b(N)})}const o=document.getElementById("btn-home");if(o){const b=N=>{N.preventDefault(),N.stopPropagation(),this.scene.resetCamera()};o.addEventListener("touchend",b),o.addEventListener("click",N=>{var U;(U=N.sourceCapabilities)!=null&&U.firesTouchEvents||b(N)})}const a=document.getElementById("size-slider"),c=document.getElementById("size-slider-phone"),h=document.getElementById("size-value"),u=document.getElementById("size-value-phone"),d=b=>{this.terrain.brushRadius=parseInt(b),h&&(h.textContent=this.terrain.brushRadius),u&&(u.textContent=this.terrain.brushRadius),a&&(a.value=this.terrain.brushRadius),c&&(c.value=this.terrain.brushRadius)};[a,c].forEach(b=>{b&&(b.value=this.terrain.brushRadius,b.addEventListener("input",N=>{N.stopPropagation(),d(N.target.value)}),b.addEventListener("touchstart",N=>N.stopPropagation()),b.addEventListener("touchmove",N=>N.stopPropagation()))}),h&&(h.textContent=this.terrain.brushRadius),u&&(u.textContent=this.terrain.brushRadius);const f=document.getElementById("strength-slider"),p=document.getElementById("strength-slider-phone"),y=document.getElementById("strength-value"),E=document.getElementById("strength-value-phone"),_=b=>{this.terrain.brushStrength=parseFloat(b),y&&(y.textContent=this.terrain.brushStrength.toFixed(1)),E&&(E.textContent=this.terrain.brushStrength.toFixed(1)),f&&(f.value=this.terrain.brushStrength),p&&(p.value=this.terrain.brushStrength)};[f,p].forEach(b=>{b&&(b.value=this.terrain.brushStrength,b.addEventListener("input",N=>{N.stopPropagation(),_(N.target.value)}),b.addEventListener("touchstart",N=>N.stopPropagation()),b.addEventListener("touchmove",N=>N.stopPropagation()))}),y&&(y.textContent=this.terrain.brushStrength.toFixed(1)),E&&(E.textContent=this.terrain.brushStrength.toFixed(1));const m=document.getElementById("btn-help-top"),C=document.getElementById("help-overlay"),w=document.getElementById("close-help");if(m&&C){const b=N=>{N.preventDefault(),N.stopPropagation(),C.classList.add("visible")};m.addEventListener("touchend",b),m.addEventListener("click",N=>{var U;(U=N.sourceCapabilities)!=null&&U.firesTouchEvents||b(N)})}if(w&&C){const b=N=>{N.preventDefault(),N.stopPropagation(),C.classList.remove("visible")};w.addEventListener("touchend",b),w.addEventListener("click",N=>{var U;(U=N.sourceCapabilities)!=null&&U.firesTouchEvents||b(N)}),C.addEventListener("touchend",N=>{N.target===C&&C.classList.remove("visible")}),C.addEventListener("click",N=>{N.target===C&&C.classList.remove("visible")})}document.querySelectorAll("#top-bar, #bottom-bar, #preset-dropdown, #quality-dropdown, .side-slider").forEach(b=>{b.addEventListener("touchstart",N=>N.stopPropagation(),{passive:!1}),b.addEventListener("touchmove",N=>N.stopPropagation(),{passive:!1})})}updateStats(t,e,n){const r=document.getElementById("stats");r&&(r.textContent=`${Math.round(t)} FPS | Water: ${e.toFixed(0)} | Springs: ${n}`)}getTouchDistance(t,e){const n=t.clientX-e.clientX,r=t.clientY-e.clientY;return Math.sqrt(n*n+r*r)}getTouchCenter(t,e){return{x:(t.clientX+e.clientX)/2,y:(t.clientY+e.clientY)/2}}getTouchAngle(t,e){return Math.atan2(e.clientY-t.clientY,e.clientX-t.clientX)}onTouchStart(t){t.preventDefault();for(const n of t.changedTouches)this.activeTouches.set(n.identifier,{x:n.clientX,y:n.clientY});const e=this.activeTouches.size;if(e===1){this.isSingleTouch=!0,this.toolAppliedThisGesture=!1;const n=t.changedTouches[0];this.applyToolAtTouch(n.clientX,n.clientY),this.toolAppliedThisGesture=!0}else if(e===2){this.isSingleTouch=!1,this.hideBrushCursor();const n=Array.from(this.activeTouches.values()),r={clientX:n[0].x,clientY:n[0].y},s={clientX:n[1].x,clientY:n[1].y};this.lastTouchDist=this.getTouchDistance(r,s),this.lastTouchCenter=this.getTouchCenter(r,s),this.lastTouchAngle=this.getTouchAngle(r,s)}}onTouchMove(t){t.preventDefault();for(const n of t.changedTouches)this.activeTouches.has(n.identifier)&&this.activeTouches.set(n.identifier,{x:n.clientX,y:n.clientY});const e=this.activeTouches.size;if(e===1&&this.isSingleTouch){const n=t.changedTouches[0];this.currentTool!==jr.SPRING&&this.applyToolAtTouch(n.clientX,n.clientY),this.updateBrushFromTouch(n.clientX,n.clientY)}else if(e===2){const n=Array.from(this.activeTouches.values()),r={clientX:n[0].x,clientY:n[0].y},s={clientX:n[1].x,clientY:n[1].y},o=this.getTouchDistance(r,s),a=this.getTouchCenter(r,s),c=this.getTouchAngle(r,s),h=o-this.lastTouchDist;if(Math.abs(h)>.5){const p=new k().subVectors(this.orbitTarget,this.camera.position).normalize(),y=h*2.5;this.camera.position.add(p.clone().multiplyScalar(y))}const u=a.x-this.lastTouchCenter.x,d=a.y-this.lastTouchCenter.y;if(Math.abs(u)>.5||Math.abs(d)>.5){const p=new k,y=new k(0,1,0);this.camera.getWorldDirection(p),p.crossVectors(y,p).normalize();const E=2.5,_=p.clone().multiplyScalar(u*E),m=y.clone().multiplyScalar(d*E);this.camera.position.add(_).add(m),this.orbitTarget.add(_).add(m)}const f=c-this.lastTouchAngle;if(Math.abs(f)>.005){const p=this.camera.position.clone().sub(this.orbitTarget),y=Math.cos(-f),E=Math.sin(-f),_=p.x*y-p.z*E,m=p.x*E+p.z*y;p.x=_,p.z=m,this.camera.position.copy(this.orbitTarget).add(p),this.camera.lookAt(this.orbitTarget)}this.lastTouchDist=o,this.lastTouchCenter=a,this.lastTouchAngle=c}}onTouchEnd(t){t.preventDefault();for(const e of t.changedTouches)this.activeTouches.delete(e.identifier);this.activeTouches.size===0?(this.isSingleTouch=!1,this.hideBrushCursor()):this.activeTouches.size===1&&(this.isSingleTouch=!1)}applyToolAtTouch(t,e){const n=this.scene.canvas.getBoundingClientRect();this.touchPoint.x=(t-n.left)/n.width*2-1,this.touchPoint.y=-((e-n.top)/n.height)*2+1,this.raycaster.setFromCamera(this.touchPoint,this.camera);const r=this.raycaster.intersectObject(this.terrainMesh.mesh);if(r.length>0){const s=r[0].point,o=s.x/this.worldScale+this.gridWidth/2,a=s.z/this.worldScale+this.gridHeight/2;switch(this.currentTool){case jr.RAISE:this.terrain.raise(o,a);break;case jr.LOWER:this.terrain.lower(o,a);break;case jr.WATER:this.onAddWater&&this.onAddWater(o,a);break;case jr.SPRING:this.onAddSpring&&this.onAddSpring(o,a);break}this.updateBrushCursor(s.x,s.y,s.z)}}updateBrushFromTouch(t,e){const n=this.scene.canvas.getBoundingClientRect();this.touchPoint.x=(t-n.left)/n.width*2-1,this.touchPoint.y=-((e-n.top)/n.height)*2+1,this.raycaster.setFromCamera(this.touchPoint,this.camera);const r=this.raycaster.intersectObject(this.terrainMesh.mesh);if(r.length>0){const s=r[0].point;this.updateBrushCursor(s.x,s.y,s.z)}}}const Rp="drift-input-mode",ln={DESKTOP:"desktop",TOUCH:"touch"};function lS(){return"ontouchstart"in window||navigator.maxTouchPoints>0||navigator.msMaxTouchPoints>0}function hS(){return window.innerWidth<=1024||window.innerHeight<=768}function Cp(){try{const i=localStorage.getItem(Rp);if(i&&(i===ln.DESKTOP||i===ln.TOUCH))return i}catch(i){console.warn("Failed to load input mode preference:",i)}return null}function uS(i){try{localStorage.setItem(Rp,i)}catch(t){console.warn("Failed to save input mode preference:",t)}}function dS(){const i=Cp();if(i)return console.log("Using saved input mode:",i),i;const t=lS(),e=hS(),n=t&&e?ln.TOUCH:ln.DESKTOP;return console.log("Auto-detected input mode:",n,{hasTouch:t,isMobile:e}),n}function Ip(){return Cp()||dS()}function fS(){const t=Ip()===ln.DESKTOP?ln.TOUCH:ln.DESKTOP;uS(t),window.location.reload()}function pS(i){return i===ln.TOUCH?{icon:"🖥️",label:"Touch",description:"Touch controls for mobile/tablet"}:{icon:"📱",label:"Desktop",description:"Mouse and keyboard controls"}}const ci={SANDBOX:"sandbox",FLOW_RUSH:"flowRush",CASCADE:"cascade",FLOOD_DEFENSE:"floodDefense"};class mS{constructor(t,e,n){this.terrain=t,this.water=e,this.scene=n,this.currentModeType=ci.SANDBOX,this.lastPlayedMode=null,this.currentMode=null,this.isPlaying=!1,this.modeClasses=new Map,this.onModeStart=null,this.onModeEnd=null,this.onScoreUpdate=null}registerMode(t,e){this.modeClasses.set(t,e)}startMode(t){this.isPlaying&&this.endMode(!1);const e=this.modeClasses.get(t);return e?(this.currentModeType=t,t!==ci.SANDBOX&&(this.lastPlayedMode=t),this.currentMode=new e({terrain:this.terrain,water:this.water,scene:this.scene,manager:this}),this.currentMode.init(),this.isPlaying=!0,this.onModeStart&&this.onModeStart(t),console.log(`Started mode: ${t}`),!0):(console.warn(`Mode "${t}" not registered`),!1)}endMode(t=!1){if(!this.isPlaying||!this.currentMode)return;const e=this.currentMode.getScore(),n=this.currentMode.getStats();return this.currentMode.cleanup(),this.currentMode=null,this.isPlaying=!1,this.onModeEnd&&this.onModeEnd(this.currentModeType,t,e,n),this.currentModeType=ci.SANDBOX,console.log(`Ended mode. Score: ${e}`),{finalScore:e,stats:n}}update(t){!this.isPlaying||!this.currentMode||(this.currentMode.update(t),this.currentMode.isGameOver()&&this.endMode(this.currentMode.isVictory()))}reportScoreUpdate(t,e){this.onScoreUpdate&&this.onScoreUpdate(t,e)}getCurrentMode(){return{type:this.currentModeType,isPlaying:this.isPlaying,mode:this.currentMode}}isSandbox(){return this.currentModeType===ci.SANDBOX}getAvailableModes(){return Array.from(this.modeClasses.keys())}}var zd={};/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */const Pp=function(i){const t=[];let e=0;for(let n=0;n<i.length;n++){let r=i.charCodeAt(n);r<128?t[e++]=r:r<2048?(t[e++]=r>>6|192,t[e++]=r&63|128):(r&64512)===55296&&n+1<i.length&&(i.charCodeAt(n+1)&64512)===56320?(r=65536+((r&1023)<<10)+(i.charCodeAt(++n)&1023),t[e++]=r>>18|240,t[e++]=r>>12&63|128,t[e++]=r>>6&63|128,t[e++]=r&63|128):(t[e++]=r>>12|224,t[e++]=r>>6&63|128,t[e++]=r&63|128)}return t},gS=function(i){const t=[];let e=0,n=0;for(;e<i.length;){const r=i[e++];if(r<128)t[n++]=String.fromCharCode(r);else if(r>191&&r<224){const s=i[e++];t[n++]=String.fromCharCode((r&31)<<6|s&63)}else if(r>239&&r<365){const s=i[e++],o=i[e++],a=i[e++],c=((r&7)<<18|(s&63)<<12|(o&63)<<6|a&63)-65536;t[n++]=String.fromCharCode(55296+(c>>10)),t[n++]=String.fromCharCode(56320+(c&1023))}else{const s=i[e++],o=i[e++];t[n++]=String.fromCharCode((r&15)<<12|(s&63)<<6|o&63)}}return t.join("")},Dp={byteToCharMap_:null,charToByteMap_:null,byteToCharMapWebSafe_:null,charToByteMapWebSafe_:null,ENCODED_VALS_BASE:"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",get ENCODED_VALS(){return this.ENCODED_VALS_BASE+"+/="},get ENCODED_VALS_WEBSAFE(){return this.ENCODED_VALS_BASE+"-_."},HAS_NATIVE_SUPPORT:typeof atob=="function",encodeByteArray(i,t){if(!Array.isArray(i))throw Error("encodeByteArray takes an array as a parameter");this.init_();const e=t?this.byteToCharMapWebSafe_:this.byteToCharMap_,n=[];for(let r=0;r<i.length;r+=3){const s=i[r],o=r+1<i.length,a=o?i[r+1]:0,c=r+2<i.length,h=c?i[r+2]:0,u=s>>2,d=(s&3)<<4|a>>4;let f=(a&15)<<2|h>>6,p=h&63;c||(p=64,o||(f=64)),n.push(e[u],e[d],e[f],e[p])}return n.join("")},encodeString(i,t){return this.HAS_NATIVE_SUPPORT&&!t?btoa(i):this.encodeByteArray(Pp(i),t)},decodeString(i,t){return this.HAS_NATIVE_SUPPORT&&!t?atob(i):gS(this.decodeStringToByteArray(i,t))},decodeStringToByteArray(i,t){this.init_();const e=t?this.charToByteMapWebSafe_:this.charToByteMap_,n=[];for(let r=0;r<i.length;){const s=e[i.charAt(r++)],a=r<i.length?e[i.charAt(r)]:0;++r;const h=r<i.length?e[i.charAt(r)]:64;++r;const d=r<i.length?e[i.charAt(r)]:64;if(++r,s==null||a==null||h==null||d==null)throw new _S;const f=s<<2|a>>4;if(n.push(f),h!==64){const p=a<<4&240|h>>2;if(n.push(p),d!==64){const y=h<<6&192|d;n.push(y)}}}return n},init_(){if(!this.byteToCharMap_){this.byteToCharMap_={},this.charToByteMap_={},this.byteToCharMapWebSafe_={},this.charToByteMapWebSafe_={};for(let i=0;i<this.ENCODED_VALS.length;i++)this.byteToCharMap_[i]=this.ENCODED_VALS.charAt(i),this.charToByteMap_[this.byteToCharMap_[i]]=i,this.byteToCharMapWebSafe_[i]=this.ENCODED_VALS_WEBSAFE.charAt(i),this.charToByteMapWebSafe_[this.byteToCharMapWebSafe_[i]]=i,i>=this.ENCODED_VALS_BASE.length&&(this.charToByteMap_[this.ENCODED_VALS_WEBSAFE.charAt(i)]=i,this.charToByteMapWebSafe_[this.ENCODED_VALS.charAt(i)]=i)}}};class _S extends Error{constructor(){super(...arguments),this.name="DecodeBase64StringError"}}const vS=function(i){const t=Pp(i);return Dp.encodeByteArray(t,!0)},Ia=function(i){return vS(i).replace(/\./g,"")},yS=function(i){try{return Dp.decodeString(i,!0)}catch(t){console.error("base64Decode failed: ",t)}return null};/**
 * @license
 * Copyright 2022 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */function ES(){if(typeof self<"u")return self;if(typeof window<"u")return window;if(typeof global<"u")return global;throw new Error("Unable to locate global object.")}/**
 * @license
 * Copyright 2022 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */const xS=()=>ES().__FIREBASE_DEFAULTS__,SS=()=>{if(typeof process>"u"||typeof zd>"u")return;const i=zd.__FIREBASE_DEFAULTS__;if(i)return JSON.parse(i)},TS=()=>{if(typeof document>"u")return;let i;try{i=document.cookie.match(/__FIREBASE_DEFAULTS__=([^;]+)/)}catch{return}const t=i&&yS(i[1]);return t&&JSON.parse(t)},Yl=()=>{try{return xS()||SS()||TS()}catch(i){console.info(`Unable to get __FIREBASE_DEFAULTS__ due to: ${i}`);return}},MS=i=>{var t,e;return(e=(t=Yl())===null||t===void 0?void 0:t.emulatorHosts)===null||e===void 0?void 0:e[i]},wS=i=>{const t=MS(i);if(!t)return;const e=t.lastIndexOf(":");if(e<=0||e+1===t.length)throw new Error(`Invalid host ${t} with no separate hostname and port!`);const n=parseInt(t.substring(e+1),10);return t[0]==="["?[t.substring(1,e-1),n]:[t.substring(0,e),n]},Lp=()=>{var i;return(i=Yl())===null||i===void 0?void 0:i.config};/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class bS{constructor(){this.reject=()=>{},this.resolve=()=>{},this.promise=new Promise((t,e)=>{this.resolve=t,this.reject=e})}wrapCallback(t){return(e,n)=>{e?this.reject(e):this.resolve(n),typeof t=="function"&&(this.promise.catch(()=>{}),t.length===1?t(e):t(e,n))}}}/**
 * @license
 * Copyright 2021 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */function AS(i,t){if(i.uid)throw new Error('The "uid" field is no longer supported by mockUserToken. Please use "sub" instead for Firebase Auth User ID.');const e={alg:"none",type:"JWT"},n=t||"demo-project",r=i.iat||0,s=i.sub||i.user_id;if(!s)throw new Error("mockUserToken must contain 'sub' or 'user_id' field!");const o=Object.assign({iss:`https://securetoken.google.com/${n}`,aud:n,iat:r,exp:r+3600,auth_time:r,sub:s,user_id:s,firebase:{sign_in_provider:"custom",identities:{}}},i);return[Ia(JSON.stringify(e)),Ia(JSON.stringify(o)),""].join(".")}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */function RS(){return typeof navigator<"u"&&typeof navigator.userAgent=="string"?navigator.userAgent:""}function CS(){var i;const t=(i=Yl())===null||i===void 0?void 0:i.forceEnvironment;if(t==="node")return!0;if(t==="browser")return!1;try{return Object.prototype.toString.call(global.process)==="[object process]"}catch{return!1}}function IS(){return!CS()&&!!navigator.userAgent&&navigator.userAgent.includes("Safari")&&!navigator.userAgent.includes("Chrome")}function PS(){try{return typeof indexedDB=="object"}catch{return!1}}function DS(){return new Promise((i,t)=>{try{let e=!0;const n="validate-browser-context-for-indexeddb-analytics-module",r=self.indexedDB.open(n);r.onsuccess=()=>{r.result.close(),e||self.indexedDB.deleteDatabase(n),i(!0)},r.onupgradeneeded=()=>{e=!1},r.onerror=()=>{var s;t(((s=r.error)===null||s===void 0?void 0:s.message)||"")}}catch(e){t(e)}})}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */const LS="FirebaseError";class ys extends Error{constructor(t,e,n){super(e),this.code=t,this.customData=n,this.name=LS,Object.setPrototypeOf(this,ys.prototype),Error.captureStackTrace&&Error.captureStackTrace(this,Np.prototype.create)}}class Np{constructor(t,e,n){this.service=t,this.serviceName=e,this.errors=n}create(t,...e){const n=e[0]||{},r=`${this.service}/${t}`,s=this.errors[t],o=s?NS(s,n):"Error",a=`${this.serviceName}: ${o} (${r}).`;return new ys(r,a,n)}}function NS(i,t){return i.replace(US,(e,n)=>{const r=t[n];return r!=null?String(r):`<${n}?>`})}const US=/\{\$([^}]+)}/g;function gl(i,t){if(i===t)return!0;const e=Object.keys(i),n=Object.keys(t);for(const r of e){if(!n.includes(r))return!1;const s=i[r],o=t[r];if(Gd(s)&&Gd(o)){if(!gl(s,o))return!1}else if(s!==o)return!1}for(const r of n)if(!e.includes(r))return!1;return!0}function Gd(i){return i!==null&&typeof i=="object"}/**
 * @license
 * Copyright 2021 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */function cs(i){return i&&i._delegate?i._delegate:i}class ro{constructor(t,e,n){this.name=t,this.instanceFactory=e,this.type=n,this.multipleInstances=!1,this.serviceProps={},this.instantiationMode="LAZY",this.onInstanceCreated=null}setInstantiationMode(t){return this.instantiationMode=t,this}setMultipleInstances(t){return this.multipleInstances=t,this}setServiceProps(t){return this.serviceProps=t,this}setInstanceCreatedCallback(t){return this.onInstanceCreated=t,this}}/**
 * @license
 * Copyright 2019 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */const er="[DEFAULT]";/**
 * @license
 * Copyright 2019 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class OS{constructor(t,e){this.name=t,this.container=e,this.component=null,this.instances=new Map,this.instancesDeferred=new Map,this.instancesOptions=new Map,this.onInitCallbacks=new Map}get(t){const e=this.normalizeInstanceIdentifier(t);if(!this.instancesDeferred.has(e)){const n=new bS;if(this.instancesDeferred.set(e,n),this.isInitialized(e)||this.shouldAutoInitialize())try{const r=this.getOrInitializeService({instanceIdentifier:e});r&&n.resolve(r)}catch{}}return this.instancesDeferred.get(e).promise}getImmediate(t){var e;const n=this.normalizeInstanceIdentifier(t==null?void 0:t.identifier),r=(e=t==null?void 0:t.optional)!==null&&e!==void 0?e:!1;if(this.isInitialized(n)||this.shouldAutoInitialize())try{return this.getOrInitializeService({instanceIdentifier:n})}catch(s){if(r)return null;throw s}else{if(r)return null;throw Error(`Service ${this.name} is not available`)}}getComponent(){return this.component}setComponent(t){if(t.name!==this.name)throw Error(`Mismatching Component ${t.name} for Provider ${this.name}.`);if(this.component)throw Error(`Component for ${this.name} has already been provided`);if(this.component=t,!!this.shouldAutoInitialize()){if(VS(t))try{this.getOrInitializeService({instanceIdentifier:er})}catch{}for(const[e,n]of this.instancesDeferred.entries()){const r=this.normalizeInstanceIdentifier(e);try{const s=this.getOrInitializeService({instanceIdentifier:r});n.resolve(s)}catch{}}}}clearInstance(t=er){this.instancesDeferred.delete(t),this.instancesOptions.delete(t),this.instances.delete(t)}async delete(){const t=Array.from(this.instances.values());await Promise.all([...t.filter(e=>"INTERNAL"in e).map(e=>e.INTERNAL.delete()),...t.filter(e=>"_delete"in e).map(e=>e._delete())])}isComponentSet(){return this.component!=null}isInitialized(t=er){return this.instances.has(t)}getOptions(t=er){return this.instancesOptions.get(t)||{}}initialize(t={}){const{options:e={}}=t,n=this.normalizeInstanceIdentifier(t.instanceIdentifier);if(this.isInitialized(n))throw Error(`${this.name}(${n}) has already been initialized`);if(!this.isComponentSet())throw Error(`Component ${this.name} has not been registered yet`);const r=this.getOrInitializeService({instanceIdentifier:n,options:e});for(const[s,o]of this.instancesDeferred.entries()){const a=this.normalizeInstanceIdentifier(s);n===a&&o.resolve(r)}return r}onInit(t,e){var n;const r=this.normalizeInstanceIdentifier(e),s=(n=this.onInitCallbacks.get(r))!==null&&n!==void 0?n:new Set;s.add(t),this.onInitCallbacks.set(r,s);const o=this.instances.get(r);return o&&t(o,r),()=>{s.delete(t)}}invokeOnInitCallbacks(t,e){const n=this.onInitCallbacks.get(e);if(n)for(const r of n)try{r(t,e)}catch{}}getOrInitializeService({instanceIdentifier:t,options:e={}}){let n=this.instances.get(t);if(!n&&this.component&&(n=this.component.instanceFactory(this.container,{instanceIdentifier:FS(t),options:e}),this.instances.set(t,n),this.instancesOptions.set(t,e),this.invokeOnInitCallbacks(n,t),this.component.onInstanceCreated))try{this.component.onInstanceCreated(this.container,t,n)}catch{}return n||null}normalizeInstanceIdentifier(t=er){return this.component?this.component.multipleInstances?t:er:t}shouldAutoInitialize(){return!!this.component&&this.component.instantiationMode!=="EXPLICIT"}}function FS(i){return i===er?void 0:i}function VS(i){return i.instantiationMode==="EAGER"}/**
 * @license
 * Copyright 2019 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class BS{constructor(t){this.name=t,this.providers=new Map}addComponent(t){const e=this.getProvider(t.name);if(e.isComponentSet())throw new Error(`Component ${t.name} has already been registered with ${this.name}`);e.setComponent(t)}addOrOverwriteComponent(t){this.getProvider(t.name).isComponentSet()&&this.providers.delete(t.name),this.addComponent(t)}getProvider(t){if(this.providers.has(t))return this.providers.get(t);const e=new OS(t,this);return this.providers.set(t,e),e}getProviders(){return Array.from(this.providers.values())}}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */var ue;(function(i){i[i.DEBUG=0]="DEBUG",i[i.VERBOSE=1]="VERBOSE",i[i.INFO=2]="INFO",i[i.WARN=3]="WARN",i[i.ERROR=4]="ERROR",i[i.SILENT=5]="SILENT"})(ue||(ue={}));const kS={debug:ue.DEBUG,verbose:ue.VERBOSE,info:ue.INFO,warn:ue.WARN,error:ue.ERROR,silent:ue.SILENT},HS=ue.INFO,zS={[ue.DEBUG]:"log",[ue.VERBOSE]:"log",[ue.INFO]:"info",[ue.WARN]:"warn",[ue.ERROR]:"error"},GS=(i,t,...e)=>{if(t<i.logLevel)return;const n=new Date().toISOString(),r=zS[t];if(r)console[r](`[${n}]  ${i.name}:`,...e);else throw new Error(`Attempted to log a message with an invalid logType (value: ${t})`)};class Up{constructor(t){this.name=t,this._logLevel=HS,this._logHandler=GS,this._userLogHandler=null}get logLevel(){return this._logLevel}set logLevel(t){if(!(t in ue))throw new TypeError(`Invalid value "${t}" assigned to \`logLevel\``);this._logLevel=t}setLogLevel(t){this._logLevel=typeof t=="string"?kS[t]:t}get logHandler(){return this._logHandler}set logHandler(t){if(typeof t!="function")throw new TypeError("Value assigned to `logHandler` must be a function");this._logHandler=t}get userLogHandler(){return this._userLogHandler}set userLogHandler(t){this._userLogHandler=t}debug(...t){this._userLogHandler&&this._userLogHandler(this,ue.DEBUG,...t),this._logHandler(this,ue.DEBUG,...t)}log(...t){this._userLogHandler&&this._userLogHandler(this,ue.VERBOSE,...t),this._logHandler(this,ue.VERBOSE,...t)}info(...t){this._userLogHandler&&this._userLogHandler(this,ue.INFO,...t),this._logHandler(this,ue.INFO,...t)}warn(...t){this._userLogHandler&&this._userLogHandler(this,ue.WARN,...t),this._logHandler(this,ue.WARN,...t)}error(...t){this._userLogHandler&&this._userLogHandler(this,ue.ERROR,...t),this._logHandler(this,ue.ERROR,...t)}}const WS=(i,t)=>t.some(e=>i instanceof e);let Wd,qd;function qS(){return Wd||(Wd=[IDBDatabase,IDBObjectStore,IDBIndex,IDBCursor,IDBTransaction])}function XS(){return qd||(qd=[IDBCursor.prototype.advance,IDBCursor.prototype.continue,IDBCursor.prototype.continuePrimaryKey])}const Op=new WeakMap,_l=new WeakMap,Fp=new WeakMap,$c=new WeakMap,$l=new WeakMap;function jS(i){const t=new Promise((e,n)=>{const r=()=>{i.removeEventListener("success",s),i.removeEventListener("error",o)},s=()=>{e(Oi(i.result)),r()},o=()=>{n(i.error),r()};i.addEventListener("success",s),i.addEventListener("error",o)});return t.then(e=>{e instanceof IDBCursor&&Op.set(e,i)}).catch(()=>{}),$l.set(t,i),t}function YS(i){if(_l.has(i))return;const t=new Promise((e,n)=>{const r=()=>{i.removeEventListener("complete",s),i.removeEventListener("error",o),i.removeEventListener("abort",o)},s=()=>{e(),r()},o=()=>{n(i.error||new DOMException("AbortError","AbortError")),r()};i.addEventListener("complete",s),i.addEventListener("error",o),i.addEventListener("abort",o)});_l.set(i,t)}let vl={get(i,t,e){if(i instanceof IDBTransaction){if(t==="done")return _l.get(i);if(t==="objectStoreNames")return i.objectStoreNames||Fp.get(i);if(t==="store")return e.objectStoreNames[1]?void 0:e.objectStore(e.objectStoreNames[0])}return Oi(i[t])},set(i,t,e){return i[t]=e,!0},has(i,t){return i instanceof IDBTransaction&&(t==="done"||t==="store")?!0:t in i}};function $S(i){vl=i(vl)}function KS(i){return i===IDBDatabase.prototype.transaction&&!("objectStoreNames"in IDBTransaction.prototype)?function(t,...e){const n=i.call(Kc(this),t,...e);return Fp.set(n,t.sort?t.sort():[t]),Oi(n)}:XS().includes(i)?function(...t){return i.apply(Kc(this),t),Oi(Op.get(this))}:function(...t){return Oi(i.apply(Kc(this),t))}}function QS(i){return typeof i=="function"?KS(i):(i instanceof IDBTransaction&&YS(i),WS(i,qS())?new Proxy(i,vl):i)}function Oi(i){if(i instanceof IDBRequest)return jS(i);if($c.has(i))return $c.get(i);const t=QS(i);return t!==i&&($c.set(i,t),$l.set(t,i)),t}const Kc=i=>$l.get(i);function ZS(i,t,{blocked:e,upgrade:n,blocking:r,terminated:s}={}){const o=indexedDB.open(i,t),a=Oi(o);return n&&o.addEventListener("upgradeneeded",c=>{n(Oi(o.result),c.oldVersion,c.newVersion,Oi(o.transaction),c)}),e&&o.addEventListener("blocked",c=>e(c.oldVersion,c.newVersion,c)),a.then(c=>{s&&c.addEventListener("close",()=>s()),r&&c.addEventListener("versionchange",h=>r(h.oldVersion,h.newVersion,h))}).catch(()=>{}),a}const JS=["get","getKey","getAll","getAllKeys","count"],tT=["put","add","delete","clear"],Qc=new Map;function Xd(i,t){if(!(i instanceof IDBDatabase&&!(t in i)&&typeof t=="string"))return;if(Qc.get(t))return Qc.get(t);const e=t.replace(/FromIndex$/,""),n=t!==e,r=tT.includes(e);if(!(e in(n?IDBIndex:IDBObjectStore).prototype)||!(r||JS.includes(e)))return;const s=async function(o,...a){const c=this.transaction(o,r?"readwrite":"readonly");let h=c.store;return n&&(h=h.index(a.shift())),(await Promise.all([h[e](...a),r&&c.done]))[0]};return Qc.set(t,s),s}$S(i=>({...i,get:(t,e,n)=>Xd(t,e)||i.get(t,e,n),has:(t,e)=>!!Xd(t,e)||i.has(t,e)}));/**
 * @license
 * Copyright 2019 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class eT{constructor(t){this.container=t}getPlatformInfoString(){return this.container.getProviders().map(e=>{if(nT(e)){const n=e.getImmediate();return`${n.library}/${n.version}`}else return null}).filter(e=>e).join(" ")}}function nT(i){const t=i.getComponent();return(t==null?void 0:t.type)==="VERSION"}const yl="@firebase/app",jd="0.10.13";/**
 * @license
 * Copyright 2019 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */const pi=new Up("@firebase/app"),iT="@firebase/app-compat",rT="@firebase/analytics-compat",sT="@firebase/analytics",oT="@firebase/app-check-compat",aT="@firebase/app-check",cT="@firebase/auth",lT="@firebase/auth-compat",hT="@firebase/database",uT="@firebase/data-connect",dT="@firebase/database-compat",fT="@firebase/functions",pT="@firebase/functions-compat",mT="@firebase/installations",gT="@firebase/installations-compat",_T="@firebase/messaging",vT="@firebase/messaging-compat",yT="@firebase/performance",ET="@firebase/performance-compat",xT="@firebase/remote-config",ST="@firebase/remote-config-compat",TT="@firebase/storage",MT="@firebase/storage-compat",wT="@firebase/firestore",bT="@firebase/vertexai-preview",AT="@firebase/firestore-compat",RT="firebase",CT="10.14.1";/**
 * @license
 * Copyright 2019 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */const El="[DEFAULT]",IT={[yl]:"fire-core",[iT]:"fire-core-compat",[sT]:"fire-analytics",[rT]:"fire-analytics-compat",[aT]:"fire-app-check",[oT]:"fire-app-check-compat",[cT]:"fire-auth",[lT]:"fire-auth-compat",[hT]:"fire-rtdb",[uT]:"fire-data-connect",[dT]:"fire-rtdb-compat",[fT]:"fire-fn",[pT]:"fire-fn-compat",[mT]:"fire-iid",[gT]:"fire-iid-compat",[_T]:"fire-fcm",[vT]:"fire-fcm-compat",[yT]:"fire-perf",[ET]:"fire-perf-compat",[xT]:"fire-rc",[ST]:"fire-rc-compat",[TT]:"fire-gcs",[MT]:"fire-gcs-compat",[wT]:"fire-fst",[AT]:"fire-fst-compat",[bT]:"fire-vertex","fire-js":"fire-js",[RT]:"fire-js-all"};/**
 * @license
 * Copyright 2019 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */const Pa=new Map,PT=new Map,xl=new Map;function Yd(i,t){try{i.container.addComponent(t)}catch(e){pi.debug(`Component ${t.name} failed to register with FirebaseApp ${i.name}`,e)}}function Da(i){const t=i.name;if(xl.has(t))return pi.debug(`There were multiple attempts to register component ${t}.`),!1;xl.set(t,i);for(const e of Pa.values())Yd(e,i);for(const e of PT.values())Yd(e,i);return!0}function DT(i,t){const e=i.container.getProvider("heartbeat").getImmediate({optional:!0});return e&&e.triggerHeartbeat(),i.container.getProvider(t)}/**
 * @license
 * Copyright 2019 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */const LT={"no-app":"No Firebase App '{$appName}' has been created - call initializeApp() first","bad-app-name":"Illegal App name: '{$appName}'","duplicate-app":"Firebase App named '{$appName}' already exists with different options or config","app-deleted":"Firebase App named '{$appName}' already deleted","server-app-deleted":"Firebase Server App has been deleted","no-options":"Need to provide options, when not being deployed to hosting via source.","invalid-app-argument":"firebase.{$appName}() takes either no argument or a Firebase App instance.","invalid-log-argument":"First argument to `onLog` must be null or a function.","idb-open":"Error thrown when opening IndexedDB. Original error: {$originalErrorMessage}.","idb-get":"Error thrown when reading from IndexedDB. Original error: {$originalErrorMessage}.","idb-set":"Error thrown when writing to IndexedDB. Original error: {$originalErrorMessage}.","idb-delete":"Error thrown when deleting from IndexedDB. Original error: {$originalErrorMessage}.","finalization-registry-not-supported":"FirebaseServerApp deleteOnDeref field defined but the JS runtime does not support FinalizationRegistry.","invalid-server-app-environment":"FirebaseServerApp is not for use in browser environments."},Fi=new Np("app","Firebase",LT);/**
 * @license
 * Copyright 2019 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class NT{constructor(t,e,n){this._isDeleted=!1,this._options=Object.assign({},t),this._config=Object.assign({},e),this._name=e.name,this._automaticDataCollectionEnabled=e.automaticDataCollectionEnabled,this._container=n,this.container.addComponent(new ro("app",()=>this,"PUBLIC"))}get automaticDataCollectionEnabled(){return this.checkDestroyed(),this._automaticDataCollectionEnabled}set automaticDataCollectionEnabled(t){this.checkDestroyed(),this._automaticDataCollectionEnabled=t}get name(){return this.checkDestroyed(),this._name}get options(){return this.checkDestroyed(),this._options}get config(){return this.checkDestroyed(),this._config}get container(){return this._container}get isDeleted(){return this._isDeleted}set isDeleted(t){this._isDeleted=t}checkDestroyed(){if(this.isDeleted)throw Fi.create("app-deleted",{appName:this._name})}}/**
 * @license
 * Copyright 2019 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */const UT=CT;function Vp(i,t={}){let e=i;typeof t!="object"&&(t={name:t});const n=Object.assign({name:El,automaticDataCollectionEnabled:!1},t),r=n.name;if(typeof r!="string"||!r)throw Fi.create("bad-app-name",{appName:String(r)});if(e||(e=Lp()),!e)throw Fi.create("no-options");const s=Pa.get(r);if(s){if(gl(e,s.options)&&gl(n,s.config))return s;throw Fi.create("duplicate-app",{appName:r})}const o=new BS(r);for(const c of xl.values())o.addComponent(c);const a=new NT(e,n,o);return Pa.set(r,a),a}function OT(i=El){const t=Pa.get(i);if(!t&&i===El&&Lp())return Vp();if(!t)throw Fi.create("no-app",{appName:i});return t}function ns(i,t,e){var n;let r=(n=IT[i])!==null&&n!==void 0?n:i;e&&(r+=`-${e}`);const s=r.match(/\s|\//),o=t.match(/\s|\//);if(s||o){const a=[`Unable to register library "${r}" with version "${t}":`];s&&a.push(`library name "${r}" contains illegal characters (whitespace or "/")`),s&&o&&a.push("and"),o&&a.push(`version name "${t}" contains illegal characters (whitespace or "/")`),pi.warn(a.join(" "));return}Da(new ro(`${r}-version`,()=>({library:r,version:t}),"VERSION"))}/**
 * @license
 * Copyright 2021 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */const FT="firebase-heartbeat-database",VT=1,so="firebase-heartbeat-store";let Zc=null;function Bp(){return Zc||(Zc=ZS(FT,VT,{upgrade:(i,t)=>{switch(t){case 0:try{i.createObjectStore(so)}catch(e){console.warn(e)}}}}).catch(i=>{throw Fi.create("idb-open",{originalErrorMessage:i.message})})),Zc}async function BT(i){try{const e=(await Bp()).transaction(so),n=await e.objectStore(so).get(kp(i));return await e.done,n}catch(t){if(t instanceof ys)pi.warn(t.message);else{const e=Fi.create("idb-get",{originalErrorMessage:t==null?void 0:t.message});pi.warn(e.message)}}}async function $d(i,t){try{const n=(await Bp()).transaction(so,"readwrite");await n.objectStore(so).put(t,kp(i)),await n.done}catch(e){if(e instanceof ys)pi.warn(e.message);else{const n=Fi.create("idb-set",{originalErrorMessage:e==null?void 0:e.message});pi.warn(n.message)}}}function kp(i){return`${i.name}!${i.options.appId}`}/**
 * @license
 * Copyright 2021 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */const kT=1024,HT=30*24*60*60*1e3;class zT{constructor(t){this.container=t,this._heartbeatsCache=null;const e=this.container.getProvider("app").getImmediate();this._storage=new WT(e),this._heartbeatsCachePromise=this._storage.read().then(n=>(this._heartbeatsCache=n,n))}async triggerHeartbeat(){var t,e;try{const r=this.container.getProvider("platform-logger").getImmediate().getPlatformInfoString(),s=Kd();return((t=this._heartbeatsCache)===null||t===void 0?void 0:t.heartbeats)==null&&(this._heartbeatsCache=await this._heartbeatsCachePromise,((e=this._heartbeatsCache)===null||e===void 0?void 0:e.heartbeats)==null)||this._heartbeatsCache.lastSentHeartbeatDate===s||this._heartbeatsCache.heartbeats.some(o=>o.date===s)?void 0:(this._heartbeatsCache.heartbeats.push({date:s,agent:r}),this._heartbeatsCache.heartbeats=this._heartbeatsCache.heartbeats.filter(o=>{const a=new Date(o.date).valueOf();return Date.now()-a<=HT}),this._storage.overwrite(this._heartbeatsCache))}catch(n){pi.warn(n)}}async getHeartbeatsHeader(){var t;try{if(this._heartbeatsCache===null&&await this._heartbeatsCachePromise,((t=this._heartbeatsCache)===null||t===void 0?void 0:t.heartbeats)==null||this._heartbeatsCache.heartbeats.length===0)return"";const e=Kd(),{heartbeatsToSend:n,unsentEntries:r}=GT(this._heartbeatsCache.heartbeats),s=Ia(JSON.stringify({version:2,heartbeats:n}));return this._heartbeatsCache.lastSentHeartbeatDate=e,r.length>0?(this._heartbeatsCache.heartbeats=r,await this._storage.overwrite(this._heartbeatsCache)):(this._heartbeatsCache.heartbeats=[],this._storage.overwrite(this._heartbeatsCache)),s}catch(e){return pi.warn(e),""}}}function Kd(){return new Date().toISOString().substring(0,10)}function GT(i,t=kT){const e=[];let n=i.slice();for(const r of i){const s=e.find(o=>o.agent===r.agent);if(s){if(s.dates.push(r.date),Qd(e)>t){s.dates.pop();break}}else if(e.push({agent:r.agent,dates:[r.date]}),Qd(e)>t){e.pop();break}n=n.slice(1)}return{heartbeatsToSend:e,unsentEntries:n}}class WT{constructor(t){this.app=t,this._canUseIndexedDBPromise=this.runIndexedDBEnvironmentCheck()}async runIndexedDBEnvironmentCheck(){return PS()?DS().then(()=>!0).catch(()=>!1):!1}async read(){if(await this._canUseIndexedDBPromise){const e=await BT(this.app);return e!=null&&e.heartbeats?e:{heartbeats:[]}}else return{heartbeats:[]}}async overwrite(t){var e;if(await this._canUseIndexedDBPromise){const r=await this.read();return $d(this.app,{lastSentHeartbeatDate:(e=t.lastSentHeartbeatDate)!==null&&e!==void 0?e:r.lastSentHeartbeatDate,heartbeats:t.heartbeats})}else return}async add(t){var e;if(await this._canUseIndexedDBPromise){const r=await this.read();return $d(this.app,{lastSentHeartbeatDate:(e=t.lastSentHeartbeatDate)!==null&&e!==void 0?e:r.lastSentHeartbeatDate,heartbeats:[...r.heartbeats,...t.heartbeats]})}else return}}function Qd(i){return Ia(JSON.stringify({version:2,heartbeats:i})).length}/**
 * @license
 * Copyright 2019 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */function qT(i){Da(new ro("platform-logger",t=>new eT(t),"PRIVATE")),Da(new ro("heartbeat",t=>new zT(t),"PRIVATE")),ns(yl,jd,i),ns(yl,jd,"esm2017"),ns("fire-js","")}qT("");var XT="firebase",jT="10.14.1";/**
 * @license
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */ns(XT,jT,"app");var Zd=typeof globalThis<"u"?globalThis:typeof window<"u"?window:typeof global<"u"?global:typeof self<"u"?self:{};/** @license
Copyright The Closure Library Authors.
SPDX-License-Identifier: Apache-2.0
*/var lr,Hp;(function(){var i;/** @license

 Copyright The Closure Library Authors.
 SPDX-License-Identifier: Apache-2.0
*/function t(T,M){function v(){}v.prototype=M.prototype,T.D=M.prototype,T.prototype=new v,T.prototype.constructor=T,T.C=function(S,P,L){for(var R=Array(arguments.length-2),V=2;V<arguments.length;V++)R[V-2]=arguments[V];return M.prototype[P].apply(S,R)}}function e(){this.blockSize=-1}function n(){this.blockSize=-1,this.blockSize=64,this.g=Array(4),this.B=Array(this.blockSize),this.o=this.h=0,this.s()}t(n,e),n.prototype.s=function(){this.g[0]=1732584193,this.g[1]=4023233417,this.g[2]=2562383102,this.g[3]=271733878,this.o=this.h=0};function r(T,M,v){v||(v=0);var S=Array(16);if(typeof M=="string")for(var P=0;16>P;++P)S[P]=M.charCodeAt(v++)|M.charCodeAt(v++)<<8|M.charCodeAt(v++)<<16|M.charCodeAt(v++)<<24;else for(P=0;16>P;++P)S[P]=M[v++]|M[v++]<<8|M[v++]<<16|M[v++]<<24;M=T.g[0],v=T.g[1],P=T.g[2];var L=T.g[3],R=M+(L^v&(P^L))+S[0]+3614090360&4294967295;M=v+(R<<7&4294967295|R>>>25),R=L+(P^M&(v^P))+S[1]+3905402710&4294967295,L=M+(R<<12&4294967295|R>>>20),R=P+(v^L&(M^v))+S[2]+606105819&4294967295,P=L+(R<<17&4294967295|R>>>15),R=v+(M^P&(L^M))+S[3]+3250441966&4294967295,v=P+(R<<22&4294967295|R>>>10),R=M+(L^v&(P^L))+S[4]+4118548399&4294967295,M=v+(R<<7&4294967295|R>>>25),R=L+(P^M&(v^P))+S[5]+1200080426&4294967295,L=M+(R<<12&4294967295|R>>>20),R=P+(v^L&(M^v))+S[6]+2821735955&4294967295,P=L+(R<<17&4294967295|R>>>15),R=v+(M^P&(L^M))+S[7]+4249261313&4294967295,v=P+(R<<22&4294967295|R>>>10),R=M+(L^v&(P^L))+S[8]+1770035416&4294967295,M=v+(R<<7&4294967295|R>>>25),R=L+(P^M&(v^P))+S[9]+2336552879&4294967295,L=M+(R<<12&4294967295|R>>>20),R=P+(v^L&(M^v))+S[10]+4294925233&4294967295,P=L+(R<<17&4294967295|R>>>15),R=v+(M^P&(L^M))+S[11]+2304563134&4294967295,v=P+(R<<22&4294967295|R>>>10),R=M+(L^v&(P^L))+S[12]+1804603682&4294967295,M=v+(R<<7&4294967295|R>>>25),R=L+(P^M&(v^P))+S[13]+4254626195&4294967295,L=M+(R<<12&4294967295|R>>>20),R=P+(v^L&(M^v))+S[14]+2792965006&4294967295,P=L+(R<<17&4294967295|R>>>15),R=v+(M^P&(L^M))+S[15]+1236535329&4294967295,v=P+(R<<22&4294967295|R>>>10),R=M+(P^L&(v^P))+S[1]+4129170786&4294967295,M=v+(R<<5&4294967295|R>>>27),R=L+(v^P&(M^v))+S[6]+3225465664&4294967295,L=M+(R<<9&4294967295|R>>>23),R=P+(M^v&(L^M))+S[11]+643717713&4294967295,P=L+(R<<14&4294967295|R>>>18),R=v+(L^M&(P^L))+S[0]+3921069994&4294967295,v=P+(R<<20&4294967295|R>>>12),R=M+(P^L&(v^P))+S[5]+3593408605&4294967295,M=v+(R<<5&4294967295|R>>>27),R=L+(v^P&(M^v))+S[10]+38016083&4294967295,L=M+(R<<9&4294967295|R>>>23),R=P+(M^v&(L^M))+S[15]+3634488961&4294967295,P=L+(R<<14&4294967295|R>>>18),R=v+(L^M&(P^L))+S[4]+3889429448&4294967295,v=P+(R<<20&4294967295|R>>>12),R=M+(P^L&(v^P))+S[9]+568446438&4294967295,M=v+(R<<5&4294967295|R>>>27),R=L+(v^P&(M^v))+S[14]+3275163606&4294967295,L=M+(R<<9&4294967295|R>>>23),R=P+(M^v&(L^M))+S[3]+4107603335&4294967295,P=L+(R<<14&4294967295|R>>>18),R=v+(L^M&(P^L))+S[8]+1163531501&4294967295,v=P+(R<<20&4294967295|R>>>12),R=M+(P^L&(v^P))+S[13]+2850285829&4294967295,M=v+(R<<5&4294967295|R>>>27),R=L+(v^P&(M^v))+S[2]+4243563512&4294967295,L=M+(R<<9&4294967295|R>>>23),R=P+(M^v&(L^M))+S[7]+1735328473&4294967295,P=L+(R<<14&4294967295|R>>>18),R=v+(L^M&(P^L))+S[12]+2368359562&4294967295,v=P+(R<<20&4294967295|R>>>12),R=M+(v^P^L)+S[5]+4294588738&4294967295,M=v+(R<<4&4294967295|R>>>28),R=L+(M^v^P)+S[8]+2272392833&4294967295,L=M+(R<<11&4294967295|R>>>21),R=P+(L^M^v)+S[11]+1839030562&4294967295,P=L+(R<<16&4294967295|R>>>16),R=v+(P^L^M)+S[14]+4259657740&4294967295,v=P+(R<<23&4294967295|R>>>9),R=M+(v^P^L)+S[1]+2763975236&4294967295,M=v+(R<<4&4294967295|R>>>28),R=L+(M^v^P)+S[4]+1272893353&4294967295,L=M+(R<<11&4294967295|R>>>21),R=P+(L^M^v)+S[7]+4139469664&4294967295,P=L+(R<<16&4294967295|R>>>16),R=v+(P^L^M)+S[10]+3200236656&4294967295,v=P+(R<<23&4294967295|R>>>9),R=M+(v^P^L)+S[13]+681279174&4294967295,M=v+(R<<4&4294967295|R>>>28),R=L+(M^v^P)+S[0]+3936430074&4294967295,L=M+(R<<11&4294967295|R>>>21),R=P+(L^M^v)+S[3]+3572445317&4294967295,P=L+(R<<16&4294967295|R>>>16),R=v+(P^L^M)+S[6]+76029189&4294967295,v=P+(R<<23&4294967295|R>>>9),R=M+(v^P^L)+S[9]+3654602809&4294967295,M=v+(R<<4&4294967295|R>>>28),R=L+(M^v^P)+S[12]+3873151461&4294967295,L=M+(R<<11&4294967295|R>>>21),R=P+(L^M^v)+S[15]+530742520&4294967295,P=L+(R<<16&4294967295|R>>>16),R=v+(P^L^M)+S[2]+3299628645&4294967295,v=P+(R<<23&4294967295|R>>>9),R=M+(P^(v|~L))+S[0]+4096336452&4294967295,M=v+(R<<6&4294967295|R>>>26),R=L+(v^(M|~P))+S[7]+1126891415&4294967295,L=M+(R<<10&4294967295|R>>>22),R=P+(M^(L|~v))+S[14]+2878612391&4294967295,P=L+(R<<15&4294967295|R>>>17),R=v+(L^(P|~M))+S[5]+4237533241&4294967295,v=P+(R<<21&4294967295|R>>>11),R=M+(P^(v|~L))+S[12]+1700485571&4294967295,M=v+(R<<6&4294967295|R>>>26),R=L+(v^(M|~P))+S[3]+2399980690&4294967295,L=M+(R<<10&4294967295|R>>>22),R=P+(M^(L|~v))+S[10]+4293915773&4294967295,P=L+(R<<15&4294967295|R>>>17),R=v+(L^(P|~M))+S[1]+2240044497&4294967295,v=P+(R<<21&4294967295|R>>>11),R=M+(P^(v|~L))+S[8]+1873313359&4294967295,M=v+(R<<6&4294967295|R>>>26),R=L+(v^(M|~P))+S[15]+4264355552&4294967295,L=M+(R<<10&4294967295|R>>>22),R=P+(M^(L|~v))+S[6]+2734768916&4294967295,P=L+(R<<15&4294967295|R>>>17),R=v+(L^(P|~M))+S[13]+1309151649&4294967295,v=P+(R<<21&4294967295|R>>>11),R=M+(P^(v|~L))+S[4]+4149444226&4294967295,M=v+(R<<6&4294967295|R>>>26),R=L+(v^(M|~P))+S[11]+3174756917&4294967295,L=M+(R<<10&4294967295|R>>>22),R=P+(M^(L|~v))+S[2]+718787259&4294967295,P=L+(R<<15&4294967295|R>>>17),R=v+(L^(P|~M))+S[9]+3951481745&4294967295,T.g[0]=T.g[0]+M&4294967295,T.g[1]=T.g[1]+(P+(R<<21&4294967295|R>>>11))&4294967295,T.g[2]=T.g[2]+P&4294967295,T.g[3]=T.g[3]+L&4294967295}n.prototype.u=function(T,M){M===void 0&&(M=T.length);for(var v=M-this.blockSize,S=this.B,P=this.h,L=0;L<M;){if(P==0)for(;L<=v;)r(this,T,L),L+=this.blockSize;if(typeof T=="string"){for(;L<M;)if(S[P++]=T.charCodeAt(L++),P==this.blockSize){r(this,S),P=0;break}}else for(;L<M;)if(S[P++]=T[L++],P==this.blockSize){r(this,S),P=0;break}}this.h=P,this.o+=M},n.prototype.v=function(){var T=Array((56>this.h?this.blockSize:2*this.blockSize)-this.h);T[0]=128;for(var M=1;M<T.length-8;++M)T[M]=0;var v=8*this.o;for(M=T.length-8;M<T.length;++M)T[M]=v&255,v/=256;for(this.u(T),T=Array(16),M=v=0;4>M;++M)for(var S=0;32>S;S+=8)T[v++]=this.g[M]>>>S&255;return T};function s(T,M){var v=a;return Object.prototype.hasOwnProperty.call(v,T)?v[T]:v[T]=M(T)}function o(T,M){this.h=M;for(var v=[],S=!0,P=T.length-1;0<=P;P--){var L=T[P]|0;S&&L==M||(v[P]=L,S=!1)}this.g=v}var a={};function c(T){return-128<=T&&128>T?s(T,function(M){return new o([M|0],0>M?-1:0)}):new o([T|0],0>T?-1:0)}function h(T){if(isNaN(T)||!isFinite(T))return d;if(0>T)return _(h(-T));for(var M=[],v=1,S=0;T>=v;S++)M[S]=T/v|0,v*=4294967296;return new o(M,0)}function u(T,M){if(T.length==0)throw Error("number format error: empty string");if(M=M||10,2>M||36<M)throw Error("radix out of range: "+M);if(T.charAt(0)=="-")return _(u(T.substring(1),M));if(0<=T.indexOf("-"))throw Error('number format error: interior "-" character');for(var v=h(Math.pow(M,8)),S=d,P=0;P<T.length;P+=8){var L=Math.min(8,T.length-P),R=parseInt(T.substring(P,P+L),M);8>L?(L=h(Math.pow(M,L)),S=S.j(L).add(h(R))):(S=S.j(v),S=S.add(h(R)))}return S}var d=c(0),f=c(1),p=c(16777216);i=o.prototype,i.m=function(){if(E(this))return-_(this).m();for(var T=0,M=1,v=0;v<this.g.length;v++){var S=this.i(v);T+=(0<=S?S:4294967296+S)*M,M*=4294967296}return T},i.toString=function(T){if(T=T||10,2>T||36<T)throw Error("radix out of range: "+T);if(y(this))return"0";if(E(this))return"-"+_(this).toString(T);for(var M=h(Math.pow(T,6)),v=this,S="";;){var P=b(v,M).g;v=m(v,P.j(M));var L=((0<v.g.length?v.g[0]:v.h)>>>0).toString(T);if(v=P,y(v))return L+S;for(;6>L.length;)L="0"+L;S=L+S}},i.i=function(T){return 0>T?0:T<this.g.length?this.g[T]:this.h};function y(T){if(T.h!=0)return!1;for(var M=0;M<T.g.length;M++)if(T.g[M]!=0)return!1;return!0}function E(T){return T.h==-1}i.l=function(T){return T=m(this,T),E(T)?-1:y(T)?0:1};function _(T){for(var M=T.g.length,v=[],S=0;S<M;S++)v[S]=~T.g[S];return new o(v,~T.h).add(f)}i.abs=function(){return E(this)?_(this):this},i.add=function(T){for(var M=Math.max(this.g.length,T.g.length),v=[],S=0,P=0;P<=M;P++){var L=S+(this.i(P)&65535)+(T.i(P)&65535),R=(L>>>16)+(this.i(P)>>>16)+(T.i(P)>>>16);S=R>>>16,L&=65535,R&=65535,v[P]=R<<16|L}return new o(v,v[v.length-1]&-2147483648?-1:0)};function m(T,M){return T.add(_(M))}i.j=function(T){if(y(this)||y(T))return d;if(E(this))return E(T)?_(this).j(_(T)):_(_(this).j(T));if(E(T))return _(this.j(_(T)));if(0>this.l(p)&&0>T.l(p))return h(this.m()*T.m());for(var M=this.g.length+T.g.length,v=[],S=0;S<2*M;S++)v[S]=0;for(S=0;S<this.g.length;S++)for(var P=0;P<T.g.length;P++){var L=this.i(S)>>>16,R=this.i(S)&65535,V=T.i(P)>>>16,G=T.i(P)&65535;v[2*S+2*P]+=R*G,C(v,2*S+2*P),v[2*S+2*P+1]+=L*G,C(v,2*S+2*P+1),v[2*S+2*P+1]+=R*V,C(v,2*S+2*P+1),v[2*S+2*P+2]+=L*V,C(v,2*S+2*P+2)}for(S=0;S<M;S++)v[S]=v[2*S+1]<<16|v[2*S];for(S=M;S<2*M;S++)v[S]=0;return new o(v,0)};function C(T,M){for(;(T[M]&65535)!=T[M];)T[M+1]+=T[M]>>>16,T[M]&=65535,M++}function w(T,M){this.g=T,this.h=M}function b(T,M){if(y(M))throw Error("division by zero");if(y(T))return new w(d,d);if(E(T))return M=b(_(T),M),new w(_(M.g),_(M.h));if(E(M))return M=b(T,_(M)),new w(_(M.g),M.h);if(30<T.g.length){if(E(T)||E(M))throw Error("slowDivide_ only works with positive integers.");for(var v=f,S=M;0>=S.l(T);)v=N(v),S=N(S);var P=U(v,1),L=U(S,1);for(S=U(S,2),v=U(v,2);!y(S);){var R=L.add(S);0>=R.l(T)&&(P=P.add(v),L=R),S=U(S,1),v=U(v,1)}return M=m(T,P.j(M)),new w(P,M)}for(P=d;0<=T.l(M);){for(v=Math.max(1,Math.floor(T.m()/M.m())),S=Math.ceil(Math.log(v)/Math.LN2),S=48>=S?1:Math.pow(2,S-48),L=h(v),R=L.j(M);E(R)||0<R.l(T);)v-=S,L=h(v),R=L.j(M);y(L)&&(L=f),P=P.add(L),T=m(T,R)}return new w(P,T)}i.A=function(T){return b(this,T).h},i.and=function(T){for(var M=Math.max(this.g.length,T.g.length),v=[],S=0;S<M;S++)v[S]=this.i(S)&T.i(S);return new o(v,this.h&T.h)},i.or=function(T){for(var M=Math.max(this.g.length,T.g.length),v=[],S=0;S<M;S++)v[S]=this.i(S)|T.i(S);return new o(v,this.h|T.h)},i.xor=function(T){for(var M=Math.max(this.g.length,T.g.length),v=[],S=0;S<M;S++)v[S]=this.i(S)^T.i(S);return new o(v,this.h^T.h)};function N(T){for(var M=T.g.length+1,v=[],S=0;S<M;S++)v[S]=T.i(S)<<1|T.i(S-1)>>>31;return new o(v,T.h)}function U(T,M){var v=M>>5;M%=32;for(var S=T.g.length-v,P=[],L=0;L<S;L++)P[L]=0<M?T.i(L+v)>>>M|T.i(L+v+1)<<32-M:T.i(L+v);return new o(P,T.h)}n.prototype.digest=n.prototype.v,n.prototype.reset=n.prototype.s,n.prototype.update=n.prototype.u,Hp=n,o.prototype.add=o.prototype.add,o.prototype.multiply=o.prototype.j,o.prototype.modulo=o.prototype.A,o.prototype.compare=o.prototype.l,o.prototype.toNumber=o.prototype.m,o.prototype.toString=o.prototype.toString,o.prototype.getBits=o.prototype.i,o.fromNumber=h,o.fromString=u,lr=o}).apply(typeof Zd<"u"?Zd:typeof self<"u"?self:typeof window<"u"?window:{});var ca=typeof globalThis<"u"?globalThis:typeof window<"u"?window:typeof global<"u"?global:typeof self<"u"?self:{};/** @license
Copyright The Closure Library Authors.
SPDX-License-Identifier: Apache-2.0
*/var zp,Ws,Gp,ma,Sl,Wp,qp,Xp;(function(){var i,t=typeof Object.defineProperties=="function"?Object.defineProperty:function(l,g,x){return l==Array.prototype||l==Object.prototype||(l[g]=x.value),l};function e(l){l=[typeof globalThis=="object"&&globalThis,l,typeof window=="object"&&window,typeof self=="object"&&self,typeof ca=="object"&&ca];for(var g=0;g<l.length;++g){var x=l[g];if(x&&x.Math==Math)return x}throw Error("Cannot find global object")}var n=e(this);function r(l,g){if(g)t:{var x=n;l=l.split(".");for(var I=0;I<l.length-1;I++){var z=l[I];if(!(z in x))break t;x=x[z]}l=l[l.length-1],I=x[l],g=g(I),g!=I&&g!=null&&t(x,l,{configurable:!0,writable:!0,value:g})}}function s(l,g){l instanceof String&&(l+="");var x=0,I=!1,z={next:function(){if(!I&&x<l.length){var Q=x++;return{value:g(Q,l[Q]),done:!1}}return I=!0,{done:!0,value:void 0}}};return z[Symbol.iterator]=function(){return z},z}r("Array.prototype.values",function(l){return l||function(){return s(this,function(g,x){return x})}});/** @license

 Copyright The Closure Library Authors.
 SPDX-License-Identifier: Apache-2.0
*/var o=o||{},a=this||self;function c(l){var g=typeof l;return g=g!="object"?g:l?Array.isArray(l)?"array":g:"null",g=="array"||g=="object"&&typeof l.length=="number"}function h(l){var g=typeof l;return g=="object"&&l!=null||g=="function"}function u(l,g,x){return l.call.apply(l.bind,arguments)}function d(l,g,x){if(!l)throw Error();if(2<arguments.length){var I=Array.prototype.slice.call(arguments,2);return function(){var z=Array.prototype.slice.call(arguments);return Array.prototype.unshift.apply(z,I),l.apply(g,z)}}return function(){return l.apply(g,arguments)}}function f(l,g,x){return f=Function.prototype.bind&&Function.prototype.bind.toString().indexOf("native code")!=-1?u:d,f.apply(null,arguments)}function p(l,g){var x=Array.prototype.slice.call(arguments,1);return function(){var I=x.slice();return I.push.apply(I,arguments),l.apply(this,I)}}function y(l,g){function x(){}x.prototype=g.prototype,l.aa=g.prototype,l.prototype=new x,l.prototype.constructor=l,l.Qb=function(I,z,Q){for(var gt=Array(arguments.length-2),Ee=2;Ee<arguments.length;Ee++)gt[Ee-2]=arguments[Ee];return g.prototype[z].apply(I,gt)}}function E(l){const g=l.length;if(0<g){const x=Array(g);for(let I=0;I<g;I++)x[I]=l[I];return x}return[]}function _(l,g){for(let x=1;x<arguments.length;x++){const I=arguments[x];if(c(I)){const z=l.length||0,Q=I.length||0;l.length=z+Q;for(let gt=0;gt<Q;gt++)l[z+gt]=I[gt]}else l.push(I)}}class m{constructor(g,x){this.i=g,this.j=x,this.h=0,this.g=null}get(){let g;return 0<this.h?(this.h--,g=this.g,this.g=g.next,g.next=null):g=this.i(),g}}function C(l){return/^[\s\xa0]*$/.test(l)}function w(){var l=a.navigator;return l&&(l=l.userAgent)?l:""}function b(l){return b[" "](l),l}b[" "]=function(){};var N=w().indexOf("Gecko")!=-1&&!(w().toLowerCase().indexOf("webkit")!=-1&&w().indexOf("Edge")==-1)&&!(w().indexOf("Trident")!=-1||w().indexOf("MSIE")!=-1)&&w().indexOf("Edge")==-1;function U(l,g,x){for(const I in l)g.call(x,l[I],I,l)}function T(l,g){for(const x in l)g.call(void 0,l[x],x,l)}function M(l){const g={};for(const x in l)g[x]=l[x];return g}const v="constructor hasOwnProperty isPrototypeOf propertyIsEnumerable toLocaleString toString valueOf".split(" ");function S(l,g){let x,I;for(let z=1;z<arguments.length;z++){I=arguments[z];for(x in I)l[x]=I[x];for(let Q=0;Q<v.length;Q++)x=v[Q],Object.prototype.hasOwnProperty.call(I,x)&&(l[x]=I[x])}}function P(l){var g=1;l=l.split(":");const x=[];for(;0<g&&l.length;)x.push(l.shift()),g--;return l.length&&x.push(l.join(":")),x}function L(l){a.setTimeout(()=>{throw l},0)}function R(){var l=et;let g=null;return l.g&&(g=l.g,l.g=l.g.next,l.g||(l.h=null),g.next=null),g}class V{constructor(){this.h=this.g=null}add(g,x){const I=G.get();I.set(g,x),this.h?this.h.next=I:this.g=I,this.h=I}}var G=new m(()=>new K,l=>l.reset());class K{constructor(){this.next=this.g=this.h=null}set(g,x){this.h=g,this.g=x,this.next=null}reset(){this.next=this.g=this.h=null}}let Z,J=!1,et=new V,tt=()=>{const l=a.Promise.resolve(void 0);Z=()=>{l.then(pt)}};var pt=()=>{for(var l;l=R();){try{l.h.call(l.g)}catch(x){L(x)}var g=G;g.j(l),100>g.h&&(g.h++,l.next=g.g,g.g=l)}J=!1};function ht(){this.s=this.s,this.C=this.C}ht.prototype.s=!1,ht.prototype.ma=function(){this.s||(this.s=!0,this.N())},ht.prototype.N=function(){if(this.C)for(;this.C.length;)this.C.shift()()};function X(l,g){this.type=l,this.g=this.target=g,this.defaultPrevented=!1}X.prototype.h=function(){this.defaultPrevented=!0};var it=function(){if(!a.addEventListener||!Object.defineProperty)return!1;var l=!1,g=Object.defineProperty({},"passive",{get:function(){l=!0}});try{const x=()=>{};a.addEventListener("test",x,g),a.removeEventListener("test",x,g)}catch{}return l}();function mt(l,g){if(X.call(this,l?l.type:""),this.relatedTarget=this.g=this.target=null,this.button=this.screenY=this.screenX=this.clientY=this.clientX=0,this.key="",this.metaKey=this.shiftKey=this.altKey=this.ctrlKey=!1,this.state=null,this.pointerId=0,this.pointerType="",this.i=null,l){var x=this.type=l.type,I=l.changedTouches&&l.changedTouches.length?l.changedTouches[0]:null;if(this.target=l.target||l.srcElement,this.g=g,g=l.relatedTarget){if(N){t:{try{b(g.nodeName);var z=!0;break t}catch{}z=!1}z||(g=null)}}else x=="mouseover"?g=l.fromElement:x=="mouseout"&&(g=l.toElement);this.relatedTarget=g,I?(this.clientX=I.clientX!==void 0?I.clientX:I.pageX,this.clientY=I.clientY!==void 0?I.clientY:I.pageY,this.screenX=I.screenX||0,this.screenY=I.screenY||0):(this.clientX=l.clientX!==void 0?l.clientX:l.pageX,this.clientY=l.clientY!==void 0?l.clientY:l.pageY,this.screenX=l.screenX||0,this.screenY=l.screenY||0),this.button=l.button,this.key=l.key||"",this.ctrlKey=l.ctrlKey,this.altKey=l.altKey,this.shiftKey=l.shiftKey,this.metaKey=l.metaKey,this.pointerId=l.pointerId||0,this.pointerType=typeof l.pointerType=="string"?l.pointerType:Pt[l.pointerType]||"",this.state=l.state,this.i=l,l.defaultPrevented&&mt.aa.h.call(this)}}y(mt,X);var Pt={2:"touch",3:"pen",4:"mouse"};mt.prototype.h=function(){mt.aa.h.call(this);var l=this.i;l.preventDefault?l.preventDefault():l.returnValue=!1};var Mt="closure_listenable_"+(1e6*Math.random()|0),Gt=0;function Ft(l,g,x,I,z){this.listener=l,this.proxy=null,this.src=g,this.type=x,this.capture=!!I,this.ha=z,this.key=++Gt,this.da=this.fa=!1}function wt(l){l.da=!0,l.listener=null,l.proxy=null,l.src=null,l.ha=null}function Wt(l){this.src=l,this.g={},this.h=0}Wt.prototype.add=function(l,g,x,I,z){var Q=l.toString();l=this.g[Q],l||(l=this.g[Q]=[],this.h++);var gt=pe(l,g,I,z);return-1<gt?(g=l[gt],x||(g.fa=!1)):(g=new Ft(g,this.src,Q,!!I,z),g.fa=x,l.push(g)),g};function W(l,g){var x=g.type;if(x in l.g){var I=l.g[x],z=Array.prototype.indexOf.call(I,g,void 0),Q;(Q=0<=z)&&Array.prototype.splice.call(I,z,1),Q&&(wt(g),l.g[x].length==0&&(delete l.g[x],l.h--))}}function pe(l,g,x,I){for(var z=0;z<l.length;++z){var Q=l[z];if(!Q.da&&Q.listener==g&&Q.capture==!!x&&Q.ha==I)return z}return-1}var Rt="closure_lm_"+(1e6*Math.random()|0),kt={};function bt(l,g,x,I,z){if(Array.isArray(g)){for(var Q=0;Q<g.length;Q++)bt(l,g[Q],x,I,z);return null}return x=Ct(x),l&&l[Mt]?l.K(g,x,h(I)?!!I.capture:!1,z):ye(l,g,x,!1,I,z)}function ye(l,g,x,I,z,Q){if(!g)throw Error("Invalid event type");var gt=h(z)?!!z.capture:!!z,Ee=rt(l);if(Ee||(l[Rt]=Ee=new Wt(l)),x=Ee.add(g,x,I,gt,Q),x.proxy)return x;if(I=te(),x.proxy=I,I.src=l,I.listener=x,l.addEventListener)it||(z=gt),z===void 0&&(z=!1),l.addEventListener(g.toString(),I,z);else if(l.attachEvent)l.attachEvent(q(g.toString()),I);else if(l.addListener&&l.removeListener)l.addListener(I);else throw Error("addEventListener and attachEvent are unavailable.");return x}function te(){function l(x){return g.call(l.src,l.listener,x)}const g=lt;return l}function O(l,g,x,I,z){if(Array.isArray(g))for(var Q=0;Q<g.length;Q++)O(l,g[Q],x,I,z);else I=h(I)?!!I.capture:!!I,x=Ct(x),l&&l[Mt]?(l=l.i,g=String(g).toString(),g in l.g&&(Q=l.g[g],x=pe(Q,x,I,z),-1<x&&(wt(Q[x]),Array.prototype.splice.call(Q,x,1),Q.length==0&&(delete l.g[g],l.h--)))):l&&(l=rt(l))&&(g=l.g[g.toString()],l=-1,g&&(l=pe(g,x,I,z)),(x=-1<l?g[l]:null)&&A(x))}function A(l){if(typeof l!="number"&&l&&!l.da){var g=l.src;if(g&&g[Mt])W(g.i,l);else{var x=l.type,I=l.proxy;g.removeEventListener?g.removeEventListener(x,I,l.capture):g.detachEvent?g.detachEvent(q(x),I):g.addListener&&g.removeListener&&g.removeListener(I),(x=rt(g))?(W(x,l),x.h==0&&(x.src=null,g[Rt]=null)):wt(l)}}}function q(l){return l in kt?kt[l]:kt[l]="on"+l}function lt(l,g){if(l.da)l=!0;else{g=new mt(g,this);var x=l.listener,I=l.ha||l.src;l.fa&&A(l),l=x.call(I,g)}return l}function rt(l){return l=l[Rt],l instanceof Wt?l:null}var ct="__closure_events_fn_"+(1e9*Math.random()>>>0);function Ct(l){return typeof l=="function"?l:(l[ct]||(l[ct]=function(g){return l.handleEvent(g)}),l[ct])}function ut(){ht.call(this),this.i=new Wt(this),this.M=this,this.F=null}y(ut,ht),ut.prototype[Mt]=!0,ut.prototype.removeEventListener=function(l,g,x,I){O(this,l,g,x,I)};function _t(l,g){var x,I=l.F;if(I)for(x=[];I;I=I.F)x.push(I);if(l=l.M,I=g.type||g,typeof g=="string")g=new X(g,l);else if(g instanceof X)g.target=g.target||l;else{var z=g;g=new X(I,l),S(g,z)}if(z=!0,x)for(var Q=x.length-1;0<=Q;Q--){var gt=g.g=x[Q];z=Nt(gt,I,!0,g)&&z}if(gt=g.g=l,z=Nt(gt,I,!0,g)&&z,z=Nt(gt,I,!1,g)&&z,x)for(Q=0;Q<x.length;Q++)gt=g.g=x[Q],z=Nt(gt,I,!1,g)&&z}ut.prototype.N=function(){if(ut.aa.N.call(this),this.i){var l=this.i,g;for(g in l.g){for(var x=l.g[g],I=0;I<x.length;I++)wt(x[I]);delete l.g[g],l.h--}}this.F=null},ut.prototype.K=function(l,g,x,I){return this.i.add(String(l),g,!1,x,I)},ut.prototype.L=function(l,g,x,I){return this.i.add(String(l),g,!0,x,I)};function Nt(l,g,x,I){if(g=l.i.g[String(g)],!g)return!0;g=g.concat();for(var z=!0,Q=0;Q<g.length;++Q){var gt=g[Q];if(gt&&!gt.da&&gt.capture==x){var Ee=gt.listener,Ke=gt.ha||gt.src;gt.fa&&W(l.i,gt),z=Ee.call(Ke,I)!==!1&&z}}return z&&!I.defaultPrevented}function Qt(l,g,x){if(typeof l=="function")x&&(l=f(l,x));else if(l&&typeof l.handleEvent=="function")l=f(l.handleEvent,l);else throw Error("Invalid listener argument");return 2147483647<Number(g)?-1:a.setTimeout(l,g||0)}function st(l){l.g=Qt(()=>{l.g=null,l.i&&(l.i=!1,st(l))},l.l);const g=l.h;l.h=null,l.m.apply(null,g)}class he extends ht{constructor(g,x){super(),this.m=g,this.l=x,this.h=null,this.i=!1,this.g=null}j(g){this.h=arguments,this.g?this.i=!0:st(this)}N(){super.N(),this.g&&(a.clearTimeout(this.g),this.g=null,this.i=!1,this.h=null)}}function Kt(l){ht.call(this),this.h=l,this.g={}}y(Kt,ht);var qt=[];function Lt(l){U(l.g,function(g,x){this.g.hasOwnProperty(x)&&A(g)},l),l.g={}}Kt.prototype.N=function(){Kt.aa.N.call(this),Lt(this)},Kt.prototype.handleEvent=function(){throw Error("EventHandler.handleEvent not implemented")};var Et=a.JSON.stringify,F=a.JSON.parse,dt=class{stringify(l){return a.JSON.stringify(l,void 0)}parse(l){return a.JSON.parse(l,void 0)}};function It(){}It.prototype.h=null;function St(l){return l.h||(l.h=l.i())}function at(){}var B={OPEN:"a",kb:"b",Ja:"c",wb:"d"};function ft(){X.call(this,"d")}y(ft,X);function yt(){X.call(this,"c")}y(yt,X);var Ut={},Ot=null;function ne(){return Ot=Ot||new ut}Ut.La="serverreachability";function se(l){X.call(this,Ut.La,l)}y(se,X);function _e(l){const g=ne();_t(g,new se(g))}Ut.STAT_EVENT="statevent";function Ae(l,g){X.call(this,Ut.STAT_EVENT,l),this.stat=g}y(Ae,X);function Xt(l){const g=ne();_t(g,new Ae(g,l))}Ut.Ma="timingevent";function Ce(l,g){X.call(this,Ut.Ma,l),this.size=g}y(Ce,X);function nn(l,g){if(typeof l!="function")throw Error("Fn must not be null and must be a function");return a.setTimeout(function(){l()},g)}function _i(){this.g=!0}_i.prototype.xa=function(){this.g=!1};function oc(l,g,x,I,z,Q){l.info(function(){if(l.g)if(Q)for(var gt="",Ee=Q.split("&"),Ke=0;Ke<Ee.length;Ke++){var de=Ee[Ke].split("=");if(1<de.length){var rn=de[0];de=de[1];var sn=rn.split("_");gt=2<=sn.length&&sn[1]=="type"?gt+(rn+"="+de+"&"):gt+(rn+"=redacted&")}}else gt=null;else gt=Q;return"XMLHTTP REQ ("+I+") [attempt "+z+"]: "+g+`
`+x+`
`+gt})}function yr(l,g,x,I,z,Q,gt){l.info(function(){return"XMLHTTP RESP ("+I+") [ attempt "+z+"]: "+g+`
`+x+`
`+Q+" "+gt})}function Jn(l,g,x,I){l.info(function(){return"XMLHTTP TEXT ("+g+"): "+So(l,x)+(I?" "+I:"")})}function Er(l,g){l.info(function(){return"TIMEOUT: "+g})}_i.prototype.info=function(){};function So(l,g){if(!l.g)return g;if(!g)return null;try{var x=JSON.parse(g);if(x){for(l=0;l<x.length;l++)if(Array.isArray(x[l])){var I=x[l];if(!(2>I.length)){var z=I[1];if(Array.isArray(z)&&!(1>z.length)){var Q=z[0];if(Q!="noop"&&Q!="stop"&&Q!="close")for(var gt=1;gt<z.length;gt++)z[gt]=""}}}}return Et(x)}catch{return g}}var Wi={NO_ERROR:0,gb:1,tb:2,sb:3,nb:4,rb:5,ub:6,Ia:7,TIMEOUT:8,xb:9},To={lb:"complete",Hb:"success",Ja:"error",Ia:"abort",zb:"ready",Ab:"readystatechange",TIMEOUT:"timeout",vb:"incrementaldata",yb:"progress",ob:"downloadprogress",Pb:"uploadprogress"},ws;function xr(){}y(xr,It),xr.prototype.g=function(){return new XMLHttpRequest},xr.prototype.i=function(){return{}},ws=new xr;function D(l,g,x,I){this.j=l,this.i=g,this.l=x,this.R=I||1,this.U=new Kt(this),this.I=45e3,this.H=null,this.o=!1,this.m=this.A=this.v=this.L=this.F=this.S=this.B=null,this.D=[],this.g=null,this.C=0,this.s=this.u=null,this.X=-1,this.J=!1,this.O=0,this.M=null,this.W=this.K=this.T=this.P=!1,this.h=new H}function H(){this.i=null,this.g="",this.h=!1}var Y={},$={};function j(l,g,x){l.L=1,l.v=Mo(Sn(g)),l.m=x,l.P=!0,xt(l,null)}function xt(l,g){l.F=Date.now(),zt(l),l.A=Sn(l.v);var x=l.A,I=l.R;Array.isArray(I)||(I=[String(I)]),Oh(x.i,"t",I),l.C=0,x=l.j.J,l.h=new H,l.g=tu(l.j,x?g:null,!l.m),0<l.O&&(l.M=new he(f(l.Y,l,l.g),l.O)),g=l.U,x=l.g,I=l.ca;var z="readystatechange";Array.isArray(z)||(z&&(qt[0]=z.toString()),z=qt);for(var Q=0;Q<z.length;Q++){var gt=bt(x,z[Q],I||g.handleEvent,!1,g.h||g);if(!gt)break;g.g[gt.key]=gt}g=l.H?M(l.H):{},l.m?(l.u||(l.u="POST"),g["Content-Type"]="application/x-www-form-urlencoded",l.g.ea(l.A,l.u,l.m,g)):(l.u="GET",l.g.ea(l.A,l.u,null,g)),_e(),oc(l.i,l.u,l.A,l.l,l.R,l.m)}D.prototype.ca=function(l){l=l.target;const g=this.M;g&&ti(l)==3?g.j():this.Y(l)},D.prototype.Y=function(l){try{if(l==this.g)t:{const sn=ti(this.g);var g=this.g.Ba();const wr=this.g.Z();if(!(3>sn)&&(sn!=3||this.g&&(this.h.h||this.g.oa()||Gh(this.g)))){this.J||sn!=4||g==7||(g==8||0>=wr?_e(3):_e(2)),Yt(this);var x=this.g.Z();this.X=x;e:if(Dt(this)){var I=Gh(this.g);l="";var z=I.length,Q=ti(this.g)==4;if(!this.h.i){if(typeof TextDecoder>"u"){me(this),jt(this);var gt="";break e}this.h.i=new a.TextDecoder}for(g=0;g<z;g++)this.h.h=!0,l+=this.h.i.decode(I[g],{stream:!(Q&&g==z-1)});I.length=0,this.h.g+=l,this.C=0,gt=this.h.g}else gt=this.g.oa();if(this.o=x==200,yr(this.i,this.u,this.A,this.l,this.R,sn,x),this.o){if(this.T&&!this.K){e:{if(this.g){var Ee,Ke=this.g;if((Ee=Ke.g?Ke.g.getResponseHeader("X-HTTP-Initial-Response"):null)&&!C(Ee)){var de=Ee;break e}}de=null}if(x=de)Jn(this.i,this.l,x,"Initial handshake response via X-HTTP-Initial-Response"),this.K=!0,$e(this,x);else{this.o=!1,this.s=3,Xt(12),me(this),jt(this);break t}}if(this.P){x=!0;let Fn;for(;!this.J&&this.C<gt.length;)if(Fn=Ht(this,gt),Fn==$){sn==4&&(this.s=4,Xt(14),x=!1),Jn(this.i,this.l,null,"[Incomplete Response]");break}else if(Fn==Y){this.s=4,Xt(15),Jn(this.i,this.l,gt,"[Invalid Chunk]"),x=!1;break}else Jn(this.i,this.l,Fn,null),$e(this,Fn);if(Dt(this)&&this.C!=0&&(this.h.g=this.h.g.slice(this.C),this.C=0),sn!=4||gt.length!=0||this.h.h||(this.s=1,Xt(16),x=!1),this.o=this.o&&x,!x)Jn(this.i,this.l,gt,"[Invalid Chunked Response]"),me(this),jt(this);else if(0<gt.length&&!this.W){this.W=!0;var rn=this.j;rn.g==this&&rn.ba&&!rn.M&&(rn.j.info("Great, no buffering proxy detected. Bytes received: "+gt.length),hc(rn),rn.M=!0,Xt(11))}}else Jn(this.i,this.l,gt,null),$e(this,gt);sn==4&&me(this),this.o&&!this.J&&(sn==4?Kh(this.j,this):(this.o=!1,zt(this)))}else Eg(this.g),x==400&&0<gt.indexOf("Unknown SID")?(this.s=3,Xt(12)):(this.s=0,Xt(13)),me(this),jt(this)}}}catch{}finally{}};function Dt(l){return l.g?l.u=="GET"&&l.L!=2&&l.j.Ca:!1}function Ht(l,g){var x=l.C,I=g.indexOf(`
`,x);return I==-1?$:(x=Number(g.substring(x,I)),isNaN(x)?Y:(I+=1,I+x>g.length?$:(g=g.slice(I,I+x),l.C=I+x,g)))}D.prototype.cancel=function(){this.J=!0,me(this)};function zt(l){l.S=Date.now()+l.I,ee(l,l.I)}function ee(l,g){if(l.B!=null)throw Error("WatchDog timer not null");l.B=nn(f(l.ba,l),g)}function Yt(l){l.B&&(a.clearTimeout(l.B),l.B=null)}D.prototype.ba=function(){this.B=null;const l=Date.now();0<=l-this.S?(Er(this.i,this.A),this.L!=2&&(_e(),Xt(17)),me(this),this.s=2,jt(this)):ee(this,this.S-l)};function jt(l){l.j.G==0||l.J||Kh(l.j,l)}function me(l){Yt(l);var g=l.M;g&&typeof g.ma=="function"&&g.ma(),l.M=null,Lt(l.U),l.g&&(g=l.g,l.g=null,g.abort(),g.ma())}function $e(l,g){try{var x=l.j;if(x.G!=0&&(x.g==l||qi(x.h,l))){if(!l.K&&qi(x.h,l)&&x.G==3){try{var I=x.Da.g.parse(g)}catch{I=null}if(Array.isArray(I)&&I.length==3){var z=I;if(z[0]==0){t:if(!x.u){if(x.g)if(x.g.F+3e3<l.F)Io(x),Ro(x);else break t;lc(x),Xt(18)}}else x.za=z[1],0<x.za-x.T&&37500>z[2]&&x.F&&x.v==0&&!x.C&&(x.C=nn(f(x.Za,x),6e3));if(1>=ie(x.h)&&x.ca){try{x.ca()}catch{}x.ca=void 0}}else Yi(x,11)}else if((l.K||x.g==l)&&Io(x),!C(g))for(z=x.Da.g.parse(g),g=0;g<z.length;g++){let de=z[g];if(x.T=de[0],de=de[1],x.G==2)if(de[0]=="c"){x.K=de[1],x.ia=de[2];const rn=de[3];rn!=null&&(x.la=rn,x.j.info("VER="+x.la));const sn=de[4];sn!=null&&(x.Aa=sn,x.j.info("SVER="+x.Aa));const wr=de[5];wr!=null&&typeof wr=="number"&&0<wr&&(I=1.5*wr,x.L=I,x.j.info("backChannelRequestTimeoutMs_="+I)),I=x;const Fn=l.g;if(Fn){const Do=Fn.g?Fn.g.getResponseHeader("X-Client-Wire-Protocol"):null;if(Do){var Q=I.h;Q.g||Do.indexOf("spdy")==-1&&Do.indexOf("quic")==-1&&Do.indexOf("h2")==-1||(Q.j=Q.l,Q.g=new Set,Q.h&&(Te(Q,Q.h),Q.h=null))}if(I.D){const uc=Fn.g?Fn.g.getResponseHeader("X-HTTP-Session-Id"):null;uc&&(I.ya=uc,Me(I.I,I.D,uc))}}x.G=3,x.l&&x.l.ua(),x.ba&&(x.R=Date.now()-l.F,x.j.info("Handshake RTT: "+x.R+"ms")),I=x;var gt=l;if(I.qa=Jh(I,I.J?I.ia:null,I.W),gt.K){jn(I.h,gt);var Ee=gt,Ke=I.L;Ke&&(Ee.I=Ke),Ee.B&&(Yt(Ee),zt(Ee)),I.g=gt}else Yh(I);0<x.i.length&&Co(x)}else de[0]!="stop"&&de[0]!="close"||Yi(x,7);else x.G==3&&(de[0]=="stop"||de[0]=="close"?de[0]=="stop"?Yi(x,7):cc(x):de[0]!="noop"&&x.l&&x.l.ta(de),x.v=0)}}_e(4)}catch{}}var Ne=class{constructor(l,g){this.g=l,this.map=g}};function An(l){this.l=l||10,a.PerformanceNavigationTiming?(l=a.performance.getEntriesByType("navigation"),l=0<l.length&&(l[0].nextHopProtocol=="hq"||l[0].nextHopProtocol=="h2")):l=!!(a.chrome&&a.chrome.loadTimes&&a.chrome.loadTimes()&&a.chrome.loadTimes().wasFetchedViaSpdy),this.j=l?this.l:1,this.g=null,1<this.j&&(this.g=new Set),this.h=null,this.i=[]}function Se(l){return l.h?!0:l.g?l.g.size>=l.j:!1}function ie(l){return l.h?1:l.g?l.g.size:0}function qi(l,g){return l.h?l.h==g:l.g?l.g.has(g):!1}function Te(l,g){l.g?l.g.add(g):l.h=g}function jn(l,g){l.h&&l.h==g?l.h=null:l.g&&l.g.has(g)&&l.g.delete(g)}An.prototype.cancel=function(){if(this.i=bs(this),this.h)this.h.cancel(),this.h=null;else if(this.g&&this.g.size!==0){for(const l of this.g.values())l.cancel();this.g.clear()}};function bs(l){if(l.h!=null)return l.i.concat(l.h.D);if(l.g!=null&&l.g.size!==0){let g=l.i;for(const x of l.g.values())g=g.concat(x.D);return g}return E(l.i)}function Xi(l){if(l.V&&typeof l.V=="function")return l.V();if(typeof Map<"u"&&l instanceof Map||typeof Set<"u"&&l instanceof Set)return Array.from(l.values());if(typeof l=="string")return l.split("");if(c(l)){for(var g=[],x=l.length,I=0;I<x;I++)g.push(l[I]);return g}g=[],x=0;for(I in l)g[x++]=l[I];return g}function As(l){if(l.na&&typeof l.na=="function")return l.na();if(!l.V||typeof l.V!="function"){if(typeof Map<"u"&&l instanceof Map)return Array.from(l.keys());if(!(typeof Set<"u"&&l instanceof Set)){if(c(l)||typeof l=="string"){var g=[];l=l.length;for(var x=0;x<l;x++)g.push(x);return g}g=[],x=0;for(const I in l)g[x++]=I;return g}}}function ke(l,g){if(l.forEach&&typeof l.forEach=="function")l.forEach(g,void 0);else if(c(l)||typeof l=="string")Array.prototype.forEach.call(l,g,void 0);else for(var x=As(l),I=Xi(l),z=I.length,Q=0;Q<z;Q++)g.call(void 0,I[Q],x&&x[Q],l)}var Yn=RegExp("^(?:([^:/?#.]+):)?(?://(?:([^\\\\/?#]*)@)?([^\\\\/?#]*?)(?::([0-9]+))?(?=[\\\\/?#]|$))?([^?#]+)?(?:\\?([^#]*))?(?:#([\\s\\S]*))?$");function Rs(l,g){if(l){l=l.split("&");for(var x=0;x<l.length;x++){var I=l[x].indexOf("="),z=null;if(0<=I){var Q=l[x].substring(0,I);z=l[x].substring(I+1)}else Q=l[x];g(Q,z?decodeURIComponent(z.replace(/\+/g," ")):"")}}}function Ue(l){if(this.g=this.o=this.j="",this.s=null,this.m=this.l="",this.h=!1,l instanceof Ue){this.h=l.h,Sr(this,l.j),this.o=l.o,this.g=l.g,ji(this,l.s),this.l=l.l;var g=l.i,x=new Ps;x.i=g.i,g.g&&(x.g=new Map(g.g),x.h=g.h),Dh(this,x),this.m=l.m}else l&&(g=String(l).match(Yn))?(this.h=!1,Sr(this,g[1]||"",!0),this.o=Cs(g[2]||""),this.g=Cs(g[3]||"",!0),ji(this,g[4]),this.l=Cs(g[5]||"",!0),Dh(this,g[6]||"",!0),this.m=Cs(g[7]||"")):(this.h=!1,this.i=new Ps(null,this.h))}Ue.prototype.toString=function(){var l=[],g=this.j;g&&l.push(Is(g,Lh,!0),":");var x=this.g;return(x||g=="file")&&(l.push("//"),(g=this.o)&&l.push(Is(g,Lh,!0),"@"),l.push(encodeURIComponent(String(x)).replace(/%25([0-9a-fA-F]{2})/g,"%$1")),x=this.s,x!=null&&l.push(":",String(x))),(x=this.l)&&(this.g&&x.charAt(0)!="/"&&l.push("/"),l.push(Is(x,x.charAt(0)=="/"?hg:lg,!0))),(x=this.i.toString())&&l.push("?",x),(x=this.m)&&l.push("#",Is(x,dg)),l.join("")};function Sn(l){return new Ue(l)}function Sr(l,g,x){l.j=x?Cs(g,!0):g,l.j&&(l.j=l.j.replace(/:$/,""))}function ji(l,g){if(g){if(g=Number(g),isNaN(g)||0>g)throw Error("Bad port number "+g);l.s=g}else l.s=null}function Dh(l,g,x){g instanceof Ps?(l.i=g,fg(l.i,l.h)):(x||(g=Is(g,ug)),l.i=new Ps(g,l.h))}function Me(l,g,x){l.i.set(g,x)}function Mo(l){return Me(l,"zx",Math.floor(2147483648*Math.random()).toString(36)+Math.abs(Math.floor(2147483648*Math.random())^Date.now()).toString(36)),l}function Cs(l,g){return l?g?decodeURI(l.replace(/%25/g,"%2525")):decodeURIComponent(l):""}function Is(l,g,x){return typeof l=="string"?(l=encodeURI(l).replace(g,cg),x&&(l=l.replace(/%25([0-9a-fA-F]{2})/g,"%$1")),l):null}function cg(l){return l=l.charCodeAt(0),"%"+(l>>4&15).toString(16)+(l&15).toString(16)}var Lh=/[#\/\?@]/g,lg=/[#\?:]/g,hg=/[#\?]/g,ug=/[#\?@]/g,dg=/#/g;function Ps(l,g){this.h=this.g=null,this.i=l||null,this.j=!!g}function vi(l){l.g||(l.g=new Map,l.h=0,l.i&&Rs(l.i,function(g,x){l.add(decodeURIComponent(g.replace(/\+/g," ")),x)}))}i=Ps.prototype,i.add=function(l,g){vi(this),this.i=null,l=Tr(this,l);var x=this.g.get(l);return x||this.g.set(l,x=[]),x.push(g),this.h+=1,this};function Nh(l,g){vi(l),g=Tr(l,g),l.g.has(g)&&(l.i=null,l.h-=l.g.get(g).length,l.g.delete(g))}function Uh(l,g){return vi(l),g=Tr(l,g),l.g.has(g)}i.forEach=function(l,g){vi(this),this.g.forEach(function(x,I){x.forEach(function(z){l.call(g,z,I,this)},this)},this)},i.na=function(){vi(this);const l=Array.from(this.g.values()),g=Array.from(this.g.keys()),x=[];for(let I=0;I<g.length;I++){const z=l[I];for(let Q=0;Q<z.length;Q++)x.push(g[I])}return x},i.V=function(l){vi(this);let g=[];if(typeof l=="string")Uh(this,l)&&(g=g.concat(this.g.get(Tr(this,l))));else{l=Array.from(this.g.values());for(let x=0;x<l.length;x++)g=g.concat(l[x])}return g},i.set=function(l,g){return vi(this),this.i=null,l=Tr(this,l),Uh(this,l)&&(this.h-=this.g.get(l).length),this.g.set(l,[g]),this.h+=1,this},i.get=function(l,g){return l?(l=this.V(l),0<l.length?String(l[0]):g):g};function Oh(l,g,x){Nh(l,g),0<x.length&&(l.i=null,l.g.set(Tr(l,g),E(x)),l.h+=x.length)}i.toString=function(){if(this.i)return this.i;if(!this.g)return"";const l=[],g=Array.from(this.g.keys());for(var x=0;x<g.length;x++){var I=g[x];const Q=encodeURIComponent(String(I)),gt=this.V(I);for(I=0;I<gt.length;I++){var z=Q;gt[I]!==""&&(z+="="+encodeURIComponent(String(gt[I]))),l.push(z)}}return this.i=l.join("&")};function Tr(l,g){return g=String(g),l.j&&(g=g.toLowerCase()),g}function fg(l,g){g&&!l.j&&(vi(l),l.i=null,l.g.forEach(function(x,I){var z=I.toLowerCase();I!=z&&(Nh(this,I),Oh(this,z,x))},l)),l.j=g}function pg(l,g){const x=new _i;if(a.Image){const I=new Image;I.onload=p(yi,x,"TestLoadImage: loaded",!0,g,I),I.onerror=p(yi,x,"TestLoadImage: error",!1,g,I),I.onabort=p(yi,x,"TestLoadImage: abort",!1,g,I),I.ontimeout=p(yi,x,"TestLoadImage: timeout",!1,g,I),a.setTimeout(function(){I.ontimeout&&I.ontimeout()},1e4),I.src=l}else g(!1)}function mg(l,g){const x=new _i,I=new AbortController,z=setTimeout(()=>{I.abort(),yi(x,"TestPingServer: timeout",!1,g)},1e4);fetch(l,{signal:I.signal}).then(Q=>{clearTimeout(z),Q.ok?yi(x,"TestPingServer: ok",!0,g):yi(x,"TestPingServer: server error",!1,g)}).catch(()=>{clearTimeout(z),yi(x,"TestPingServer: error",!1,g)})}function yi(l,g,x,I,z){try{z&&(z.onload=null,z.onerror=null,z.onabort=null,z.ontimeout=null),I(x)}catch{}}function gg(){this.g=new dt}function _g(l,g,x){const I=x||"";try{ke(l,function(z,Q){let gt=z;h(z)&&(gt=Et(z)),g.push(I+Q+"="+encodeURIComponent(gt))})}catch(z){throw g.push(I+"type="+encodeURIComponent("_badmap")),z}}function wo(l){this.l=l.Ub||null,this.j=l.eb||!1}y(wo,It),wo.prototype.g=function(){return new bo(this.l,this.j)},wo.prototype.i=function(l){return function(){return l}}({});function bo(l,g){ut.call(this),this.D=l,this.o=g,this.m=void 0,this.status=this.readyState=0,this.responseType=this.responseText=this.response=this.statusText="",this.onreadystatechange=null,this.u=new Headers,this.h=null,this.B="GET",this.A="",this.g=!1,this.v=this.j=this.l=null}y(bo,ut),i=bo.prototype,i.open=function(l,g){if(this.readyState!=0)throw this.abort(),Error("Error reopening a connection");this.B=l,this.A=g,this.readyState=1,Ls(this)},i.send=function(l){if(this.readyState!=1)throw this.abort(),Error("need to call open() first. ");this.g=!0;const g={headers:this.u,method:this.B,credentials:this.m,cache:void 0};l&&(g.body=l),(this.D||a).fetch(new Request(this.A,g)).then(this.Sa.bind(this),this.ga.bind(this))},i.abort=function(){this.response=this.responseText="",this.u=new Headers,this.status=0,this.j&&this.j.cancel("Request was aborted.").catch(()=>{}),1<=this.readyState&&this.g&&this.readyState!=4&&(this.g=!1,Ds(this)),this.readyState=0},i.Sa=function(l){if(this.g&&(this.l=l,this.h||(this.status=this.l.status,this.statusText=this.l.statusText,this.h=l.headers,this.readyState=2,Ls(this)),this.g&&(this.readyState=3,Ls(this),this.g)))if(this.responseType==="arraybuffer")l.arrayBuffer().then(this.Qa.bind(this),this.ga.bind(this));else if(typeof a.ReadableStream<"u"&&"body"in l){if(this.j=l.body.getReader(),this.o){if(this.responseType)throw Error('responseType must be empty for "streamBinaryChunks" mode responses.');this.response=[]}else this.response=this.responseText="",this.v=new TextDecoder;Fh(this)}else l.text().then(this.Ra.bind(this),this.ga.bind(this))};function Fh(l){l.j.read().then(l.Pa.bind(l)).catch(l.ga.bind(l))}i.Pa=function(l){if(this.g){if(this.o&&l.value)this.response.push(l.value);else if(!this.o){var g=l.value?l.value:new Uint8Array(0);(g=this.v.decode(g,{stream:!l.done}))&&(this.response=this.responseText+=g)}l.done?Ds(this):Ls(this),this.readyState==3&&Fh(this)}},i.Ra=function(l){this.g&&(this.response=this.responseText=l,Ds(this))},i.Qa=function(l){this.g&&(this.response=l,Ds(this))},i.ga=function(){this.g&&Ds(this)};function Ds(l){l.readyState=4,l.l=null,l.j=null,l.v=null,Ls(l)}i.setRequestHeader=function(l,g){this.u.append(l,g)},i.getResponseHeader=function(l){return this.h&&this.h.get(l.toLowerCase())||""},i.getAllResponseHeaders=function(){if(!this.h)return"";const l=[],g=this.h.entries();for(var x=g.next();!x.done;)x=x.value,l.push(x[0]+": "+x[1]),x=g.next();return l.join(`\r
`)};function Ls(l){l.onreadystatechange&&l.onreadystatechange.call(l)}Object.defineProperty(bo.prototype,"withCredentials",{get:function(){return this.m==="include"},set:function(l){this.m=l?"include":"same-origin"}});function Vh(l){let g="";return U(l,function(x,I){g+=I,g+=":",g+=x,g+=`\r
`}),g}function ac(l,g,x){t:{for(I in x){var I=!1;break t}I=!0}I||(x=Vh(x),typeof l=="string"?x!=null&&encodeURIComponent(String(x)):Me(l,g,x))}function De(l){ut.call(this),this.headers=new Map,this.o=l||null,this.h=!1,this.v=this.g=null,this.D="",this.m=0,this.l="",this.j=this.B=this.u=this.A=!1,this.I=null,this.H="",this.J=!1}y(De,ut);var vg=/^https?$/i,yg=["POST","PUT"];i=De.prototype,i.Ha=function(l){this.J=l},i.ea=function(l,g,x,I){if(this.g)throw Error("[goog.net.XhrIo] Object is active with another request="+this.D+"; newUri="+l);g=g?g.toUpperCase():"GET",this.D=l,this.l="",this.m=0,this.A=!1,this.h=!0,this.g=this.o?this.o.g():ws.g(),this.v=this.o?St(this.o):St(ws),this.g.onreadystatechange=f(this.Ea,this);try{this.B=!0,this.g.open(g,String(l),!0),this.B=!1}catch(Q){Bh(this,Q);return}if(l=x||"",x=new Map(this.headers),I)if(Object.getPrototypeOf(I)===Object.prototype)for(var z in I)x.set(z,I[z]);else if(typeof I.keys=="function"&&typeof I.get=="function")for(const Q of I.keys())x.set(Q,I.get(Q));else throw Error("Unknown input type for opt_headers: "+String(I));I=Array.from(x.keys()).find(Q=>Q.toLowerCase()=="content-type"),z=a.FormData&&l instanceof a.FormData,!(0<=Array.prototype.indexOf.call(yg,g,void 0))||I||z||x.set("Content-Type","application/x-www-form-urlencoded;charset=utf-8");for(const[Q,gt]of x)this.g.setRequestHeader(Q,gt);this.H&&(this.g.responseType=this.H),"withCredentials"in this.g&&this.g.withCredentials!==this.J&&(this.g.withCredentials=this.J);try{zh(this),this.u=!0,this.g.send(l),this.u=!1}catch(Q){Bh(this,Q)}};function Bh(l,g){l.h=!1,l.g&&(l.j=!0,l.g.abort(),l.j=!1),l.l=g,l.m=5,kh(l),Ao(l)}function kh(l){l.A||(l.A=!0,_t(l,"complete"),_t(l,"error"))}i.abort=function(l){this.g&&this.h&&(this.h=!1,this.j=!0,this.g.abort(),this.j=!1,this.m=l||7,_t(this,"complete"),_t(this,"abort"),Ao(this))},i.N=function(){this.g&&(this.h&&(this.h=!1,this.j=!0,this.g.abort(),this.j=!1),Ao(this,!0)),De.aa.N.call(this)},i.Ea=function(){this.s||(this.B||this.u||this.j?Hh(this):this.bb())},i.bb=function(){Hh(this)};function Hh(l){if(l.h&&typeof o<"u"&&(!l.v[1]||ti(l)!=4||l.Z()!=2)){if(l.u&&ti(l)==4)Qt(l.Ea,0,l);else if(_t(l,"readystatechange"),ti(l)==4){l.h=!1;try{const gt=l.Z();t:switch(gt){case 200:case 201:case 202:case 204:case 206:case 304:case 1223:var g=!0;break t;default:g=!1}var x;if(!(x=g)){var I;if(I=gt===0){var z=String(l.D).match(Yn)[1]||null;!z&&a.self&&a.self.location&&(z=a.self.location.protocol.slice(0,-1)),I=!vg.test(z?z.toLowerCase():"")}x=I}if(x)_t(l,"complete"),_t(l,"success");else{l.m=6;try{var Q=2<ti(l)?l.g.statusText:""}catch{Q=""}l.l=Q+" ["+l.Z()+"]",kh(l)}}finally{Ao(l)}}}}function Ao(l,g){if(l.g){zh(l);const x=l.g,I=l.v[0]?()=>{}:null;l.g=null,l.v=null,g||_t(l,"ready");try{x.onreadystatechange=I}catch{}}}function zh(l){l.I&&(a.clearTimeout(l.I),l.I=null)}i.isActive=function(){return!!this.g};function ti(l){return l.g?l.g.readyState:0}i.Z=function(){try{return 2<ti(this)?this.g.status:-1}catch{return-1}},i.oa=function(){try{return this.g?this.g.responseText:""}catch{return""}},i.Oa=function(l){if(this.g){var g=this.g.responseText;return l&&g.indexOf(l)==0&&(g=g.substring(l.length)),F(g)}};function Gh(l){try{if(!l.g)return null;if("response"in l.g)return l.g.response;switch(l.H){case"":case"text":return l.g.responseText;case"arraybuffer":if("mozResponseArrayBuffer"in l.g)return l.g.mozResponseArrayBuffer}return null}catch{return null}}function Eg(l){const g={};l=(l.g&&2<=ti(l)&&l.g.getAllResponseHeaders()||"").split(`\r
`);for(let I=0;I<l.length;I++){if(C(l[I]))continue;var x=P(l[I]);const z=x[0];if(x=x[1],typeof x!="string")continue;x=x.trim();const Q=g[z]||[];g[z]=Q,Q.push(x)}T(g,function(I){return I.join(", ")})}i.Ba=function(){return this.m},i.Ka=function(){return typeof this.l=="string"?this.l:String(this.l)};function Ns(l,g,x){return x&&x.internalChannelParams&&x.internalChannelParams[l]||g}function Wh(l){this.Aa=0,this.i=[],this.j=new _i,this.ia=this.qa=this.I=this.W=this.g=this.ya=this.D=this.H=this.m=this.S=this.o=null,this.Ya=this.U=0,this.Va=Ns("failFast",!1,l),this.F=this.C=this.u=this.s=this.l=null,this.X=!0,this.za=this.T=-1,this.Y=this.v=this.B=0,this.Ta=Ns("baseRetryDelayMs",5e3,l),this.cb=Ns("retryDelaySeedMs",1e4,l),this.Wa=Ns("forwardChannelMaxRetries",2,l),this.wa=Ns("forwardChannelRequestTimeoutMs",2e4,l),this.pa=l&&l.xmlHttpFactory||void 0,this.Xa=l&&l.Tb||void 0,this.Ca=l&&l.useFetchStreams||!1,this.L=void 0,this.J=l&&l.supportsCrossDomainXhr||!1,this.K="",this.h=new An(l&&l.concurrentRequestLimit),this.Da=new gg,this.P=l&&l.fastHandshake||!1,this.O=l&&l.encodeInitMessageHeaders||!1,this.P&&this.O&&(this.O=!1),this.Ua=l&&l.Rb||!1,l&&l.xa&&this.j.xa(),l&&l.forceLongPolling&&(this.X=!1),this.ba=!this.P&&this.X&&l&&l.detectBufferingProxy||!1,this.ja=void 0,l&&l.longPollingTimeout&&0<l.longPollingTimeout&&(this.ja=l.longPollingTimeout),this.ca=void 0,this.R=0,this.M=!1,this.ka=this.A=null}i=Wh.prototype,i.la=8,i.G=1,i.connect=function(l,g,x,I){Xt(0),this.W=l,this.H=g||{},x&&I!==void 0&&(this.H.OSID=x,this.H.OAID=I),this.F=this.X,this.I=Jh(this,null,this.W),Co(this)};function cc(l){if(qh(l),l.G==3){var g=l.U++,x=Sn(l.I);if(Me(x,"SID",l.K),Me(x,"RID",g),Me(x,"TYPE","terminate"),Us(l,x),g=new D(l,l.j,g),g.L=2,g.v=Mo(Sn(x)),x=!1,a.navigator&&a.navigator.sendBeacon)try{x=a.navigator.sendBeacon(g.v.toString(),"")}catch{}!x&&a.Image&&(new Image().src=g.v,x=!0),x||(g.g=tu(g.j,null),g.g.ea(g.v)),g.F=Date.now(),zt(g)}Zh(l)}function Ro(l){l.g&&(hc(l),l.g.cancel(),l.g=null)}function qh(l){Ro(l),l.u&&(a.clearTimeout(l.u),l.u=null),Io(l),l.h.cancel(),l.s&&(typeof l.s=="number"&&a.clearTimeout(l.s),l.s=null)}function Co(l){if(!Se(l.h)&&!l.s){l.s=!0;var g=l.Ga;Z||tt(),J||(Z(),J=!0),et.add(g,l),l.B=0}}function xg(l,g){return ie(l.h)>=l.h.j-(l.s?1:0)?!1:l.s?(l.i=g.D.concat(l.i),!0):l.G==1||l.G==2||l.B>=(l.Va?0:l.Wa)?!1:(l.s=nn(f(l.Ga,l,g),Qh(l,l.B)),l.B++,!0)}i.Ga=function(l){if(this.s)if(this.s=null,this.G==1){if(!l){this.U=Math.floor(1e5*Math.random()),l=this.U++;const z=new D(this,this.j,l);let Q=this.o;if(this.S&&(Q?(Q=M(Q),S(Q,this.S)):Q=this.S),this.m!==null||this.O||(z.H=Q,Q=null),this.P)t:{for(var g=0,x=0;x<this.i.length;x++){e:{var I=this.i[x];if("__data__"in I.map&&(I=I.map.__data__,typeof I=="string")){I=I.length;break e}I=void 0}if(I===void 0)break;if(g+=I,4096<g){g=x;break t}if(g===4096||x===this.i.length-1){g=x+1;break t}}g=1e3}else g=1e3;g=jh(this,z,g),x=Sn(this.I),Me(x,"RID",l),Me(x,"CVER",22),this.D&&Me(x,"X-HTTP-Session-Id",this.D),Us(this,x),Q&&(this.O?g="headers="+encodeURIComponent(String(Vh(Q)))+"&"+g:this.m&&ac(x,this.m,Q)),Te(this.h,z),this.Ua&&Me(x,"TYPE","init"),this.P?(Me(x,"$req",g),Me(x,"SID","null"),z.T=!0,j(z,x,null)):j(z,x,g),this.G=2}}else this.G==3&&(l?Xh(this,l):this.i.length==0||Se(this.h)||Xh(this))};function Xh(l,g){var x;g?x=g.l:x=l.U++;const I=Sn(l.I);Me(I,"SID",l.K),Me(I,"RID",x),Me(I,"AID",l.T),Us(l,I),l.m&&l.o&&ac(I,l.m,l.o),x=new D(l,l.j,x,l.B+1),l.m===null&&(x.H=l.o),g&&(l.i=g.D.concat(l.i)),g=jh(l,x,1e3),x.I=Math.round(.5*l.wa)+Math.round(.5*l.wa*Math.random()),Te(l.h,x),j(x,I,g)}function Us(l,g){l.H&&U(l.H,function(x,I){Me(g,I,x)}),l.l&&ke({},function(x,I){Me(g,I,x)})}function jh(l,g,x){x=Math.min(l.i.length,x);var I=l.l?f(l.l.Na,l.l,l):null;t:{var z=l.i;let Q=-1;for(;;){const gt=["count="+x];Q==-1?0<x?(Q=z[0].g,gt.push("ofs="+Q)):Q=0:gt.push("ofs="+Q);let Ee=!0;for(let Ke=0;Ke<x;Ke++){let de=z[Ke].g;const rn=z[Ke].map;if(de-=Q,0>de)Q=Math.max(0,z[Ke].g-100),Ee=!1;else try{_g(rn,gt,"req"+de+"_")}catch{I&&I(rn)}}if(Ee){I=gt.join("&");break t}}}return l=l.i.splice(0,x),g.D=l,I}function Yh(l){if(!l.g&&!l.u){l.Y=1;var g=l.Fa;Z||tt(),J||(Z(),J=!0),et.add(g,l),l.v=0}}function lc(l){return l.g||l.u||3<=l.v?!1:(l.Y++,l.u=nn(f(l.Fa,l),Qh(l,l.v)),l.v++,!0)}i.Fa=function(){if(this.u=null,$h(this),this.ba&&!(this.M||this.g==null||0>=this.R)){var l=2*this.R;this.j.info("BP detection timer enabled: "+l),this.A=nn(f(this.ab,this),l)}},i.ab=function(){this.A&&(this.A=null,this.j.info("BP detection timeout reached."),this.j.info("Buffering proxy detected and switch to long-polling!"),this.F=!1,this.M=!0,Xt(10),Ro(this),$h(this))};function hc(l){l.A!=null&&(a.clearTimeout(l.A),l.A=null)}function $h(l){l.g=new D(l,l.j,"rpc",l.Y),l.m===null&&(l.g.H=l.o),l.g.O=0;var g=Sn(l.qa);Me(g,"RID","rpc"),Me(g,"SID",l.K),Me(g,"AID",l.T),Me(g,"CI",l.F?"0":"1"),!l.F&&l.ja&&Me(g,"TO",l.ja),Me(g,"TYPE","xmlhttp"),Us(l,g),l.m&&l.o&&ac(g,l.m,l.o),l.L&&(l.g.I=l.L);var x=l.g;l=l.ia,x.L=1,x.v=Mo(Sn(g)),x.m=null,x.P=!0,xt(x,l)}i.Za=function(){this.C!=null&&(this.C=null,Ro(this),lc(this),Xt(19))};function Io(l){l.C!=null&&(a.clearTimeout(l.C),l.C=null)}function Kh(l,g){var x=null;if(l.g==g){Io(l),hc(l),l.g=null;var I=2}else if(qi(l.h,g))x=g.D,jn(l.h,g),I=1;else return;if(l.G!=0){if(g.o)if(I==1){x=g.m?g.m.length:0,g=Date.now()-g.F;var z=l.B;I=ne(),_t(I,new Ce(I,x)),Co(l)}else Yh(l);else if(z=g.s,z==3||z==0&&0<g.X||!(I==1&&xg(l,g)||I==2&&lc(l)))switch(x&&0<x.length&&(g=l.h,g.i=g.i.concat(x)),z){case 1:Yi(l,5);break;case 4:Yi(l,10);break;case 3:Yi(l,6);break;default:Yi(l,2)}}}function Qh(l,g){let x=l.Ta+Math.floor(Math.random()*l.cb);return l.isActive()||(x*=2),x*g}function Yi(l,g){if(l.j.info("Error code "+g),g==2){var x=f(l.fb,l),I=l.Xa;const z=!I;I=new Ue(I||"//www.google.com/images/cleardot.gif"),a.location&&a.location.protocol=="http"||Sr(I,"https"),Mo(I),z?pg(I.toString(),x):mg(I.toString(),x)}else Xt(2);l.G=0,l.l&&l.l.sa(g),Zh(l),qh(l)}i.fb=function(l){l?(this.j.info("Successfully pinged google.com"),Xt(2)):(this.j.info("Failed to ping google.com"),Xt(1))};function Zh(l){if(l.G=0,l.ka=[],l.l){const g=bs(l.h);(g.length!=0||l.i.length!=0)&&(_(l.ka,g),_(l.ka,l.i),l.h.i.length=0,E(l.i),l.i.length=0),l.l.ra()}}function Jh(l,g,x){var I=x instanceof Ue?Sn(x):new Ue(x);if(I.g!="")g&&(I.g=g+"."+I.g),ji(I,I.s);else{var z=a.location;I=z.protocol,g=g?g+"."+z.hostname:z.hostname,z=+z.port;var Q=new Ue(null);I&&Sr(Q,I),g&&(Q.g=g),z&&ji(Q,z),x&&(Q.l=x),I=Q}return x=l.D,g=l.ya,x&&g&&Me(I,x,g),Me(I,"VER",l.la),Us(l,I),I}function tu(l,g,x){if(g&&!l.J)throw Error("Can't create secondary domain capable XhrIo object.");return g=l.Ca&&!l.pa?new De(new wo({eb:x})):new De(l.pa),g.Ha(l.J),g}i.isActive=function(){return!!this.l&&this.l.isActive(this)};function eu(){}i=eu.prototype,i.ua=function(){},i.ta=function(){},i.sa=function(){},i.ra=function(){},i.isActive=function(){return!0},i.Na=function(){};function Po(){}Po.prototype.g=function(l,g){return new Tn(l,g)};function Tn(l,g){ut.call(this),this.g=new Wh(g),this.l=l,this.h=g&&g.messageUrlParams||null,l=g&&g.messageHeaders||null,g&&g.clientProtocolHeaderRequired&&(l?l["X-Client-Protocol"]="webchannel":l={"X-Client-Protocol":"webchannel"}),this.g.o=l,l=g&&g.initMessageHeaders||null,g&&g.messageContentType&&(l?l["X-WebChannel-Content-Type"]=g.messageContentType:l={"X-WebChannel-Content-Type":g.messageContentType}),g&&g.va&&(l?l["X-WebChannel-Client-Profile"]=g.va:l={"X-WebChannel-Client-Profile":g.va}),this.g.S=l,(l=g&&g.Sb)&&!C(l)&&(this.g.m=l),this.v=g&&g.supportsCrossDomainXhr||!1,this.u=g&&g.sendRawJson||!1,(g=g&&g.httpSessionIdParam)&&!C(g)&&(this.g.D=g,l=this.h,l!==null&&g in l&&(l=this.h,g in l&&delete l[g])),this.j=new Mr(this)}y(Tn,ut),Tn.prototype.m=function(){this.g.l=this.j,this.v&&(this.g.J=!0),this.g.connect(this.l,this.h||void 0)},Tn.prototype.close=function(){cc(this.g)},Tn.prototype.o=function(l){var g=this.g;if(typeof l=="string"){var x={};x.__data__=l,l=x}else this.u&&(x={},x.__data__=Et(l),l=x);g.i.push(new Ne(g.Ya++,l)),g.G==3&&Co(g)},Tn.prototype.N=function(){this.g.l=null,delete this.j,cc(this.g),delete this.g,Tn.aa.N.call(this)};function nu(l){ft.call(this),l.__headers__&&(this.headers=l.__headers__,this.statusCode=l.__status__,delete l.__headers__,delete l.__status__);var g=l.__sm__;if(g){t:{for(const x in g){l=x;break t}l=void 0}(this.i=l)&&(l=this.i,g=g!==null&&l in g?g[l]:void 0),this.data=g}else this.data=l}y(nu,ft);function iu(){yt.call(this),this.status=1}y(iu,yt);function Mr(l){this.g=l}y(Mr,eu),Mr.prototype.ua=function(){_t(this.g,"a")},Mr.prototype.ta=function(l){_t(this.g,new nu(l))},Mr.prototype.sa=function(l){_t(this.g,new iu)},Mr.prototype.ra=function(){_t(this.g,"b")},Po.prototype.createWebChannel=Po.prototype.g,Tn.prototype.send=Tn.prototype.o,Tn.prototype.open=Tn.prototype.m,Tn.prototype.close=Tn.prototype.close,Xp=function(){return new Po},qp=function(){return ne()},Wp=Ut,Sl={mb:0,pb:1,qb:2,Jb:3,Ob:4,Lb:5,Mb:6,Kb:7,Ib:8,Nb:9,PROXY:10,NOPROXY:11,Gb:12,Cb:13,Db:14,Bb:15,Eb:16,Fb:17,ib:18,hb:19,jb:20},Wi.NO_ERROR=0,Wi.TIMEOUT=8,Wi.HTTP_ERROR=6,ma=Wi,To.COMPLETE="complete",Gp=To,at.EventType=B,B.OPEN="a",B.CLOSE="b",B.ERROR="c",B.MESSAGE="d",ut.prototype.listen=ut.prototype.K,Ws=at,De.prototype.listenOnce=De.prototype.L,De.prototype.getLastError=De.prototype.Ka,De.prototype.getLastErrorCode=De.prototype.Ba,De.prototype.getStatus=De.prototype.Z,De.prototype.getResponseJson=De.prototype.Oa,De.prototype.getResponseText=De.prototype.oa,De.prototype.send=De.prototype.ea,De.prototype.setWithCredentials=De.prototype.Ha,zp=De}).apply(typeof ca<"u"?ca:typeof self<"u"?self:typeof window<"u"?window:{});const Jd="@firebase/firestore";/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class hn{constructor(t){this.uid=t}isAuthenticated(){return this.uid!=null}toKey(){return this.isAuthenticated()?"uid:"+this.uid:"anonymous-user"}isEqual(t){return t.uid===this.uid}}hn.UNAUTHENTICATED=new hn(null),hn.GOOGLE_CREDENTIALS=new hn("google-credentials-uid"),hn.FIRST_PARTY=new hn("first-party-uid"),hn.MOCK_USER=new hn("mock-user");/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */let Es="10.14.0";/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */const fr=new Up("@firebase/firestore");function Gs(){return fr.logLevel}function At(i,...t){if(fr.logLevel<=ue.DEBUG){const e=t.map(Kl);fr.debug(`Firestore (${Es}): ${i}`,...e)}}function mi(i,...t){if(fr.logLevel<=ue.ERROR){const e=t.map(Kl);fr.error(`Firestore (${Es}): ${i}`,...e)}}function ls(i,...t){if(fr.logLevel<=ue.WARN){const e=t.map(Kl);fr.warn(`Firestore (${Es}): ${i}`,...e)}}function Kl(i){if(typeof i=="string")return i;try{/**
* @license
* Copyright 2020 Google LLC
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*   http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/return function(e){return JSON.stringify(e)}(i)}catch{return i}}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */function $t(i="Unexpected state"){const t=`FIRESTORE (${Es}) INTERNAL ASSERTION FAILED: `+i;throw mi(t),new Error(t)}function ge(i,t){i||$t()}function Jt(i,t){return i}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */const nt={OK:"ok",CANCELLED:"cancelled",UNKNOWN:"unknown",INVALID_ARGUMENT:"invalid-argument",DEADLINE_EXCEEDED:"deadline-exceeded",NOT_FOUND:"not-found",ALREADY_EXISTS:"already-exists",PERMISSION_DENIED:"permission-denied",UNAUTHENTICATED:"unauthenticated",RESOURCE_EXHAUSTED:"resource-exhausted",FAILED_PRECONDITION:"failed-precondition",ABORTED:"aborted",OUT_OF_RANGE:"out-of-range",UNIMPLEMENTED:"unimplemented",INTERNAL:"internal",UNAVAILABLE:"unavailable",DATA_LOSS:"data-loss"};class Tt extends ys{constructor(t,e){super(t,e),this.code=t,this.message=e,this.toString=()=>`${this.name}: [code=${this.code}]: ${this.message}`}}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class Vi{constructor(){this.promise=new Promise((t,e)=>{this.resolve=t,this.reject=e})}}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class jp{constructor(t,e){this.user=e,this.type="OAuth",this.headers=new Map,this.headers.set("Authorization",`Bearer ${t}`)}}class YT{getToken(){return Promise.resolve(null)}invalidateToken(){}start(t,e){t.enqueueRetryable(()=>e(hn.UNAUTHENTICATED))}shutdown(){}}class $T{constructor(t){this.token=t,this.changeListener=null}getToken(){return Promise.resolve(this.token)}invalidateToken(){}start(t,e){this.changeListener=e,t.enqueueRetryable(()=>e(this.token.user))}shutdown(){this.changeListener=null}}class KT{constructor(t){this.t=t,this.currentUser=hn.UNAUTHENTICATED,this.i=0,this.forceRefresh=!1,this.auth=null}start(t,e){ge(this.o===void 0);let n=this.i;const r=c=>this.i!==n?(n=this.i,e(c)):Promise.resolve();let s=new Vi;this.o=()=>{this.i++,this.currentUser=this.u(),s.resolve(),s=new Vi,t.enqueueRetryable(()=>r(this.currentUser))};const o=()=>{const c=s;t.enqueueRetryable(async()=>{await c.promise,await r(this.currentUser)})},a=c=>{At("FirebaseAuthCredentialsProvider","Auth detected"),this.auth=c,this.o&&(this.auth.addAuthTokenListener(this.o),o())};this.t.onInit(c=>a(c)),setTimeout(()=>{if(!this.auth){const c=this.t.getImmediate({optional:!0});c?a(c):(At("FirebaseAuthCredentialsProvider","Auth not yet detected"),s.resolve(),s=new Vi)}},0),o()}getToken(){const t=this.i,e=this.forceRefresh;return this.forceRefresh=!1,this.auth?this.auth.getToken(e).then(n=>this.i!==t?(At("FirebaseAuthCredentialsProvider","getToken aborted due to token change."),this.getToken()):n?(ge(typeof n.accessToken=="string"),new jp(n.accessToken,this.currentUser)):null):Promise.resolve(null)}invalidateToken(){this.forceRefresh=!0}shutdown(){this.auth&&this.o&&this.auth.removeAuthTokenListener(this.o),this.o=void 0}u(){const t=this.auth&&this.auth.getUid();return ge(t===null||typeof t=="string"),new hn(t)}}class QT{constructor(t,e,n){this.l=t,this.h=e,this.P=n,this.type="FirstParty",this.user=hn.FIRST_PARTY,this.I=new Map}T(){return this.P?this.P():null}get headers(){this.I.set("X-Goog-AuthUser",this.l);const t=this.T();return t&&this.I.set("Authorization",t),this.h&&this.I.set("X-Goog-Iam-Authorization-Token",this.h),this.I}}class ZT{constructor(t,e,n){this.l=t,this.h=e,this.P=n}getToken(){return Promise.resolve(new QT(this.l,this.h,this.P))}start(t,e){t.enqueueRetryable(()=>e(hn.FIRST_PARTY))}shutdown(){}invalidateToken(){}}class JT{constructor(t){this.value=t,this.type="AppCheck",this.headers=new Map,t&&t.length>0&&this.headers.set("x-firebase-appcheck",this.value)}}class tM{constructor(t){this.A=t,this.forceRefresh=!1,this.appCheck=null,this.R=null}start(t,e){ge(this.o===void 0);const n=s=>{s.error!=null&&At("FirebaseAppCheckTokenProvider",`Error getting App Check token; using placeholder token instead. Error: ${s.error.message}`);const o=s.token!==this.R;return this.R=s.token,At("FirebaseAppCheckTokenProvider",`Received ${o?"new":"existing"} token.`),o?e(s.token):Promise.resolve()};this.o=s=>{t.enqueueRetryable(()=>n(s))};const r=s=>{At("FirebaseAppCheckTokenProvider","AppCheck detected"),this.appCheck=s,this.o&&this.appCheck.addTokenListener(this.o)};this.A.onInit(s=>r(s)),setTimeout(()=>{if(!this.appCheck){const s=this.A.getImmediate({optional:!0});s?r(s):At("FirebaseAppCheckTokenProvider","AppCheck not yet detected")}},0)}getToken(){const t=this.forceRefresh;return this.forceRefresh=!1,this.appCheck?this.appCheck.getToken(t).then(e=>e?(ge(typeof e.token=="string"),this.R=e.token,new JT(e.token)):null):Promise.resolve(null)}invalidateToken(){this.forceRefresh=!0}shutdown(){this.appCheck&&this.o&&this.appCheck.removeTokenListener(this.o),this.o=void 0}}/**
 * @license
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */function eM(i){const t=typeof self<"u"&&(self.crypto||self.msCrypto),e=new Uint8Array(i);if(t&&typeof t.getRandomValues=="function")t.getRandomValues(e);else for(let n=0;n<i;n++)e[n]=Math.floor(256*Math.random());return e}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class Yp{static newId(){const t="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",e=Math.floor(256/t.length)*t.length;let n="";for(;n.length<20;){const r=eM(40);for(let s=0;s<r.length;++s)n.length<20&&r[s]<e&&(n+=t.charAt(r[s]%t.length))}return n}}function fe(i,t){return i<t?-1:i>t?1:0}function hs(i,t,e){return i.length===t.length&&i.every((n,r)=>e(n,t[r]))}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class He{constructor(t,e){if(this.seconds=t,this.nanoseconds=e,e<0)throw new Tt(nt.INVALID_ARGUMENT,"Timestamp nanoseconds out of range: "+e);if(e>=1e9)throw new Tt(nt.INVALID_ARGUMENT,"Timestamp nanoseconds out of range: "+e);if(t<-62135596800)throw new Tt(nt.INVALID_ARGUMENT,"Timestamp seconds out of range: "+t);if(t>=253402300800)throw new Tt(nt.INVALID_ARGUMENT,"Timestamp seconds out of range: "+t)}static now(){return He.fromMillis(Date.now())}static fromDate(t){return He.fromMillis(t.getTime())}static fromMillis(t){const e=Math.floor(t/1e3),n=Math.floor(1e6*(t-1e3*e));return new He(e,n)}toDate(){return new Date(this.toMillis())}toMillis(){return 1e3*this.seconds+this.nanoseconds/1e6}_compareTo(t){return this.seconds===t.seconds?fe(this.nanoseconds,t.nanoseconds):fe(this.seconds,t.seconds)}isEqual(t){return t.seconds===this.seconds&&t.nanoseconds===this.nanoseconds}toString(){return"Timestamp(seconds="+this.seconds+", nanoseconds="+this.nanoseconds+")"}toJSON(){return{seconds:this.seconds,nanoseconds:this.nanoseconds}}valueOf(){const t=this.seconds- -62135596800;return String(t).padStart(12,"0")+"."+String(this.nanoseconds).padStart(9,"0")}}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class Zt{constructor(t){this.timestamp=t}static fromTimestamp(t){return new Zt(t)}static min(){return new Zt(new He(0,0))}static max(){return new Zt(new He(253402300799,999999999))}compareTo(t){return this.timestamp._compareTo(t.timestamp)}isEqual(t){return this.timestamp.isEqual(t.timestamp)}toMicroseconds(){return 1e6*this.timestamp.seconds+this.timestamp.nanoseconds/1e3}toString(){return"SnapshotVersion("+this.timestamp.toString()+")"}toTimestamp(){return this.timestamp}}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class oo{constructor(t,e,n){e===void 0?e=0:e>t.length&&$t(),n===void 0?n=t.length-e:n>t.length-e&&$t(),this.segments=t,this.offset=e,this.len=n}get length(){return this.len}isEqual(t){return oo.comparator(this,t)===0}child(t){const e=this.segments.slice(this.offset,this.limit());return t instanceof oo?t.forEach(n=>{e.push(n)}):e.push(t),this.construct(e)}limit(){return this.offset+this.length}popFirst(t){return t=t===void 0?1:t,this.construct(this.segments,this.offset+t,this.length-t)}popLast(){return this.construct(this.segments,this.offset,this.length-1)}firstSegment(){return this.segments[this.offset]}lastSegment(){return this.get(this.length-1)}get(t){return this.segments[this.offset+t]}isEmpty(){return this.length===0}isPrefixOf(t){if(t.length<this.length)return!1;for(let e=0;e<this.length;e++)if(this.get(e)!==t.get(e))return!1;return!0}isImmediateParentOf(t){if(this.length+1!==t.length)return!1;for(let e=0;e<this.length;e++)if(this.get(e)!==t.get(e))return!1;return!0}forEach(t){for(let e=this.offset,n=this.limit();e<n;e++)t(this.segments[e])}toArray(){return this.segments.slice(this.offset,this.limit())}static comparator(t,e){const n=Math.min(t.length,e.length);for(let r=0;r<n;r++){const s=t.get(r),o=e.get(r);if(s<o)return-1;if(s>o)return 1}return t.length<e.length?-1:t.length>e.length?1:0}}class be extends oo{construct(t,e,n){return new be(t,e,n)}canonicalString(){return this.toArray().join("/")}toString(){return this.canonicalString()}toUriEncodedString(){return this.toArray().map(encodeURIComponent).join("/")}static fromString(...t){const e=[];for(const n of t){if(n.indexOf("//")>=0)throw new Tt(nt.INVALID_ARGUMENT,`Invalid segment (${n}). Paths must not contain // in them.`);e.push(...n.split("/").filter(r=>r.length>0))}return new be(e)}static emptyPath(){return new be([])}}const nM=/^[_a-zA-Z][_a-zA-Z0-9]*$/;class Je extends oo{construct(t,e,n){return new Je(t,e,n)}static isValidIdentifier(t){return nM.test(t)}canonicalString(){return this.toArray().map(t=>(t=t.replace(/\\/g,"\\\\").replace(/`/g,"\\`"),Je.isValidIdentifier(t)||(t="`"+t+"`"),t)).join(".")}toString(){return this.canonicalString()}isKeyField(){return this.length===1&&this.get(0)==="__name__"}static keyField(){return new Je(["__name__"])}static fromServerFormat(t){const e=[];let n="",r=0;const s=()=>{if(n.length===0)throw new Tt(nt.INVALID_ARGUMENT,`Invalid field path (${t}). Paths must not be empty, begin with '.', end with '.', or contain '..'`);e.push(n),n=""};let o=!1;for(;r<t.length;){const a=t[r];if(a==="\\"){if(r+1===t.length)throw new Tt(nt.INVALID_ARGUMENT,"Path has trailing escape character: "+t);const c=t[r+1];if(c!=="\\"&&c!=="."&&c!=="`")throw new Tt(nt.INVALID_ARGUMENT,"Path has invalid escape sequence: "+t);n+=c,r+=2}else a==="`"?(o=!o,r++):a!=="."||o?(n+=a,r++):(s(),r++)}if(s(),o)throw new Tt(nt.INVALID_ARGUMENT,"Unterminated ` in path: "+t);return new Je(e)}static emptyPath(){return new Je([])}}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class Bt{constructor(t){this.path=t}static fromPath(t){return new Bt(be.fromString(t))}static fromName(t){return new Bt(be.fromString(t).popFirst(5))}static empty(){return new Bt(be.emptyPath())}get collectionGroup(){return this.path.popLast().lastSegment()}hasCollectionId(t){return this.path.length>=2&&this.path.get(this.path.length-2)===t}getCollectionGroup(){return this.path.get(this.path.length-2)}getCollectionPath(){return this.path.popLast()}isEqual(t){return t!==null&&be.comparator(this.path,t.path)===0}toString(){return this.path.toString()}static comparator(t,e){return be.comparator(t.path,e.path)}static isDocumentKey(t){return t.length%2==0}static fromSegments(t){return new Bt(new be(t.slice()))}}function iM(i,t){const e=i.toTimestamp().seconds,n=i.toTimestamp().nanoseconds+1,r=Zt.fromTimestamp(n===1e9?new He(e+1,0):new He(e,n));return new ki(r,Bt.empty(),t)}function rM(i){return new ki(i.readTime,i.key,-1)}class ki{constructor(t,e,n){this.readTime=t,this.documentKey=e,this.largestBatchId=n}static min(){return new ki(Zt.min(),Bt.empty(),-1)}static max(){return new ki(Zt.max(),Bt.empty(),-1)}}function sM(i,t){let e=i.readTime.compareTo(t.readTime);return e!==0?e:(e=Bt.comparator(i.documentKey,t.documentKey),e!==0?e:fe(i.largestBatchId,t.largestBatchId))}/**
 * @license
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */const oM="The current tab is not in the required state to perform this operation. It might be necessary to refresh the browser tab.";class aM{constructor(){this.onCommittedListeners=[]}addOnCommittedListener(t){this.onCommittedListeners.push(t)}raiseOnCommittedEvent(){this.onCommittedListeners.forEach(t=>t())}}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */async function go(i){if(i.code!==nt.FAILED_PRECONDITION||i.message!==oM)throw i;At("LocalStore","Unexpectedly lost primary lease")}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class ot{constructor(t){this.nextCallback=null,this.catchCallback=null,this.result=void 0,this.error=void 0,this.isDone=!1,this.callbackAttached=!1,t(e=>{this.isDone=!0,this.result=e,this.nextCallback&&this.nextCallback(e)},e=>{this.isDone=!0,this.error=e,this.catchCallback&&this.catchCallback(e)})}catch(t){return this.next(void 0,t)}next(t,e){return this.callbackAttached&&$t(),this.callbackAttached=!0,this.isDone?this.error?this.wrapFailure(e,this.error):this.wrapSuccess(t,this.result):new ot((n,r)=>{this.nextCallback=s=>{this.wrapSuccess(t,s).next(n,r)},this.catchCallback=s=>{this.wrapFailure(e,s).next(n,r)}})}toPromise(){return new Promise((t,e)=>{this.next(t,e)})}wrapUserFunction(t){try{const e=t();return e instanceof ot?e:ot.resolve(e)}catch(e){return ot.reject(e)}}wrapSuccess(t,e){return t?this.wrapUserFunction(()=>t(e)):ot.resolve(e)}wrapFailure(t,e){return t?this.wrapUserFunction(()=>t(e)):ot.reject(e)}static resolve(t){return new ot((e,n)=>{e(t)})}static reject(t){return new ot((e,n)=>{n(t)})}static waitFor(t){return new ot((e,n)=>{let r=0,s=0,o=!1;t.forEach(a=>{++r,a.next(()=>{++s,o&&s===r&&e()},c=>n(c))}),o=!0,s===r&&e()})}static or(t){let e=ot.resolve(!1);for(const n of t)e=e.next(r=>r?ot.resolve(r):n());return e}static forEach(t,e){const n=[];return t.forEach((r,s)=>{n.push(e.call(this,r,s))}),this.waitFor(n)}static mapArray(t,e){return new ot((n,r)=>{const s=t.length,o=new Array(s);let a=0;for(let c=0;c<s;c++){const h=c;e(t[h]).next(u=>{o[h]=u,++a,a===s&&n(o)},u=>r(u))}})}static doWhile(t,e){return new ot((n,r)=>{const s=()=>{t()===!0?e().next(()=>{s()},r):n()};s()})}}function cM(i){const t=i.match(/Android ([\d.]+)/i),e=t?t[1].split(".").slice(0,2).join("."):"-1";return Number(e)}function _o(i){return i.name==="IndexedDbTransactionError"}/**
 * @license
 * Copyright 2018 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class Ql{constructor(t,e){this.previousValue=t,e&&(e.sequenceNumberHandler=n=>this.ie(n),this.se=n=>e.writeSequenceNumber(n))}ie(t){return this.previousValue=Math.max(t,this.previousValue),this.previousValue}next(){const t=++this.previousValue;return this.se&&this.se(t),t}}Ql.oe=-1;function $a(i){return i==null}function La(i){return i===0&&1/i==-1/0}function lM(i){return typeof i=="number"&&Number.isInteger(i)&&!La(i)&&i<=Number.MAX_SAFE_INTEGER&&i>=Number.MIN_SAFE_INTEGER}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */function tf(i){let t=0;for(const e in i)Object.prototype.hasOwnProperty.call(i,e)&&t++;return t}function xs(i,t){for(const e in i)Object.prototype.hasOwnProperty.call(i,e)&&t(e,i[e])}function $p(i){for(const t in i)if(Object.prototype.hasOwnProperty.call(i,t))return!1;return!0}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class Pe{constructor(t,e){this.comparator=t,this.root=e||Qe.EMPTY}insert(t,e){return new Pe(this.comparator,this.root.insert(t,e,this.comparator).copy(null,null,Qe.BLACK,null,null))}remove(t){return new Pe(this.comparator,this.root.remove(t,this.comparator).copy(null,null,Qe.BLACK,null,null))}get(t){let e=this.root;for(;!e.isEmpty();){const n=this.comparator(t,e.key);if(n===0)return e.value;n<0?e=e.left:n>0&&(e=e.right)}return null}indexOf(t){let e=0,n=this.root;for(;!n.isEmpty();){const r=this.comparator(t,n.key);if(r===0)return e+n.left.size;r<0?n=n.left:(e+=n.left.size+1,n=n.right)}return-1}isEmpty(){return this.root.isEmpty()}get size(){return this.root.size}minKey(){return this.root.minKey()}maxKey(){return this.root.maxKey()}inorderTraversal(t){return this.root.inorderTraversal(t)}forEach(t){this.inorderTraversal((e,n)=>(t(e,n),!1))}toString(){const t=[];return this.inorderTraversal((e,n)=>(t.push(`${e}:${n}`),!1)),`{${t.join(", ")}}`}reverseTraversal(t){return this.root.reverseTraversal(t)}getIterator(){return new la(this.root,null,this.comparator,!1)}getIteratorFrom(t){return new la(this.root,t,this.comparator,!1)}getReverseIterator(){return new la(this.root,null,this.comparator,!0)}getReverseIteratorFrom(t){return new la(this.root,t,this.comparator,!0)}}class la{constructor(t,e,n,r){this.isReverse=r,this.nodeStack=[];let s=1;for(;!t.isEmpty();)if(s=e?n(t.key,e):1,e&&r&&(s*=-1),s<0)t=this.isReverse?t.left:t.right;else{if(s===0){this.nodeStack.push(t);break}this.nodeStack.push(t),t=this.isReverse?t.right:t.left}}getNext(){let t=this.nodeStack.pop();const e={key:t.key,value:t.value};if(this.isReverse)for(t=t.left;!t.isEmpty();)this.nodeStack.push(t),t=t.right;else for(t=t.right;!t.isEmpty();)this.nodeStack.push(t),t=t.left;return e}hasNext(){return this.nodeStack.length>0}peek(){if(this.nodeStack.length===0)return null;const t=this.nodeStack[this.nodeStack.length-1];return{key:t.key,value:t.value}}}class Qe{constructor(t,e,n,r,s){this.key=t,this.value=e,this.color=n??Qe.RED,this.left=r??Qe.EMPTY,this.right=s??Qe.EMPTY,this.size=this.left.size+1+this.right.size}copy(t,e,n,r,s){return new Qe(t??this.key,e??this.value,n??this.color,r??this.left,s??this.right)}isEmpty(){return!1}inorderTraversal(t){return this.left.inorderTraversal(t)||t(this.key,this.value)||this.right.inorderTraversal(t)}reverseTraversal(t){return this.right.reverseTraversal(t)||t(this.key,this.value)||this.left.reverseTraversal(t)}min(){return this.left.isEmpty()?this:this.left.min()}minKey(){return this.min().key}maxKey(){return this.right.isEmpty()?this.key:this.right.maxKey()}insert(t,e,n){let r=this;const s=n(t,r.key);return r=s<0?r.copy(null,null,null,r.left.insert(t,e,n),null):s===0?r.copy(null,e,null,null,null):r.copy(null,null,null,null,r.right.insert(t,e,n)),r.fixUp()}removeMin(){if(this.left.isEmpty())return Qe.EMPTY;let t=this;return t.left.isRed()||t.left.left.isRed()||(t=t.moveRedLeft()),t=t.copy(null,null,null,t.left.removeMin(),null),t.fixUp()}remove(t,e){let n,r=this;if(e(t,r.key)<0)r.left.isEmpty()||r.left.isRed()||r.left.left.isRed()||(r=r.moveRedLeft()),r=r.copy(null,null,null,r.left.remove(t,e),null);else{if(r.left.isRed()&&(r=r.rotateRight()),r.right.isEmpty()||r.right.isRed()||r.right.left.isRed()||(r=r.moveRedRight()),e(t,r.key)===0){if(r.right.isEmpty())return Qe.EMPTY;n=r.right.min(),r=r.copy(n.key,n.value,null,null,r.right.removeMin())}r=r.copy(null,null,null,null,r.right.remove(t,e))}return r.fixUp()}isRed(){return this.color}fixUp(){let t=this;return t.right.isRed()&&!t.left.isRed()&&(t=t.rotateLeft()),t.left.isRed()&&t.left.left.isRed()&&(t=t.rotateRight()),t.left.isRed()&&t.right.isRed()&&(t=t.colorFlip()),t}moveRedLeft(){let t=this.colorFlip();return t.right.left.isRed()&&(t=t.copy(null,null,null,null,t.right.rotateRight()),t=t.rotateLeft(),t=t.colorFlip()),t}moveRedRight(){let t=this.colorFlip();return t.left.left.isRed()&&(t=t.rotateRight(),t=t.colorFlip()),t}rotateLeft(){const t=this.copy(null,null,Qe.RED,null,this.right.left);return this.right.copy(null,null,this.color,t,null)}rotateRight(){const t=this.copy(null,null,Qe.RED,this.left.right,null);return this.left.copy(null,null,this.color,null,t)}colorFlip(){const t=this.left.copy(null,null,!this.left.color,null,null),e=this.right.copy(null,null,!this.right.color,null,null);return this.copy(null,null,!this.color,t,e)}checkMaxDepth(){const t=this.check();return Math.pow(2,t)<=this.size+1}check(){if(this.isRed()&&this.left.isRed()||this.right.isRed())throw $t();const t=this.left.check();if(t!==this.right.check())throw $t();return t+(this.isRed()?0:1)}}Qe.EMPTY=null,Qe.RED=!0,Qe.BLACK=!1;Qe.EMPTY=new class{constructor(){this.size=0}get key(){throw $t()}get value(){throw $t()}get color(){throw $t()}get left(){throw $t()}get right(){throw $t()}copy(t,e,n,r,s){return this}insert(t,e,n){return new Qe(t,e)}remove(t,e){return this}isEmpty(){return!0}inorderTraversal(t){return!1}reverseTraversal(t){return!1}minKey(){return null}maxKey(){return null}isRed(){return!1}checkMaxDepth(){return!0}check(){return 0}};/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class tn{constructor(t){this.comparator=t,this.data=new Pe(this.comparator)}has(t){return this.data.get(t)!==null}first(){return this.data.minKey()}last(){return this.data.maxKey()}get size(){return this.data.size}indexOf(t){return this.data.indexOf(t)}forEach(t){this.data.inorderTraversal((e,n)=>(t(e),!1))}forEachInRange(t,e){const n=this.data.getIteratorFrom(t[0]);for(;n.hasNext();){const r=n.getNext();if(this.comparator(r.key,t[1])>=0)return;e(r.key)}}forEachWhile(t,e){let n;for(n=e!==void 0?this.data.getIteratorFrom(e):this.data.getIterator();n.hasNext();)if(!t(n.getNext().key))return}firstAfterOrEqual(t){const e=this.data.getIteratorFrom(t);return e.hasNext()?e.getNext().key:null}getIterator(){return new ef(this.data.getIterator())}getIteratorFrom(t){return new ef(this.data.getIteratorFrom(t))}add(t){return this.copy(this.data.remove(t).insert(t,!0))}delete(t){return this.has(t)?this.copy(this.data.remove(t)):this}isEmpty(){return this.data.isEmpty()}unionWith(t){let e=this;return e.size<t.size&&(e=t,t=this),t.forEach(n=>{e=e.add(n)}),e}isEqual(t){if(!(t instanceof tn)||this.size!==t.size)return!1;const e=this.data.getIterator(),n=t.data.getIterator();for(;e.hasNext();){const r=e.getNext().key,s=n.getNext().key;if(this.comparator(r,s)!==0)return!1}return!0}toArray(){const t=[];return this.forEach(e=>{t.push(e)}),t}toString(){const t=[];return this.forEach(e=>t.push(e)),"SortedSet("+t.toString()+")"}copy(t){const e=new tn(this.comparator);return e.data=t,e}}class ef{constructor(t){this.iter=t}getNext(){return this.iter.getNext().key}hasNext(){return this.iter.hasNext()}}/**
 * @license
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class qn{constructor(t){this.fields=t,t.sort(Je.comparator)}static empty(){return new qn([])}unionWith(t){let e=new tn(Je.comparator);for(const n of this.fields)e=e.add(n);for(const n of t)e=e.add(n);return new qn(e.toArray())}covers(t){for(const e of this.fields)if(e.isPrefixOf(t))return!0;return!1}isEqual(t){return hs(this.fields,t.fields,(e,n)=>e.isEqual(n))}}/**
 * @license
 * Copyright 2023 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class Kp extends Error{constructor(){super(...arguments),this.name="Base64DecodeError"}}/**
 * @license
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class en{constructor(t){this.binaryString=t}static fromBase64String(t){const e=function(r){try{return atob(r)}catch(s){throw typeof DOMException<"u"&&s instanceof DOMException?new Kp("Invalid base64 string: "+s):s}}(t);return new en(e)}static fromUint8Array(t){const e=function(r){let s="";for(let o=0;o<r.length;++o)s+=String.fromCharCode(r[o]);return s}(t);return new en(e)}[Symbol.iterator](){let t=0;return{next:()=>t<this.binaryString.length?{value:this.binaryString.charCodeAt(t++),done:!1}:{value:void 0,done:!0}}}toBase64(){return function(e){return btoa(e)}(this.binaryString)}toUint8Array(){return function(e){const n=new Uint8Array(e.length);for(let r=0;r<e.length;r++)n[r]=e.charCodeAt(r);return n}(this.binaryString)}approximateByteSize(){return 2*this.binaryString.length}compareTo(t){return fe(this.binaryString,t.binaryString)}isEqual(t){return this.binaryString===t.binaryString}}en.EMPTY_BYTE_STRING=new en("");const hM=new RegExp(/^\d{4}-\d\d-\d\dT\d\d:\d\d:\d\d(?:\.(\d+))?Z$/);function Hi(i){if(ge(!!i),typeof i=="string"){let t=0;const e=hM.exec(i);if(ge(!!e),e[1]){let r=e[1];r=(r+"000000000").substr(0,9),t=Number(r)}const n=new Date(i);return{seconds:Math.floor(n.getTime()/1e3),nanos:t}}return{seconds:Le(i.seconds),nanos:Le(i.nanos)}}function Le(i){return typeof i=="number"?i:typeof i=="string"?Number(i):0}function pr(i){return typeof i=="string"?en.fromBase64String(i):en.fromUint8Array(i)}/**
 * @license
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */function Zl(i){var t,e;return((e=(((t=i==null?void 0:i.mapValue)===null||t===void 0?void 0:t.fields)||{}).__type__)===null||e===void 0?void 0:e.stringValue)==="server_timestamp"}function Jl(i){const t=i.mapValue.fields.__previous_value__;return Zl(t)?Jl(t):t}function ao(i){const t=Hi(i.mapValue.fields.__local_write_time__.timestampValue);return new He(t.seconds,t.nanos)}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class uM{constructor(t,e,n,r,s,o,a,c,h){this.databaseId=t,this.appId=e,this.persistenceKey=n,this.host=r,this.ssl=s,this.forceLongPolling=o,this.autoDetectLongPolling=a,this.longPollingOptions=c,this.useFetchStreams=h}}class co{constructor(t,e){this.projectId=t,this.database=e||"(default)"}static empty(){return new co("","")}get isDefaultDatabase(){return this.database==="(default)"}isEqual(t){return t instanceof co&&t.projectId===this.projectId&&t.database===this.database}}/**
 * @license
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */const ha={mapValue:{}};function mr(i){return"nullValue"in i?0:"booleanValue"in i?1:"integerValue"in i||"doubleValue"in i?2:"timestampValue"in i?3:"stringValue"in i?5:"bytesValue"in i?6:"referenceValue"in i?7:"geoPointValue"in i?8:"arrayValue"in i?9:"mapValue"in i?Zl(i)?4:fM(i)?9007199254740991:dM(i)?10:11:$t()}function Zn(i,t){if(i===t)return!0;const e=mr(i);if(e!==mr(t))return!1;switch(e){case 0:case 9007199254740991:return!0;case 1:return i.booleanValue===t.booleanValue;case 4:return ao(i).isEqual(ao(t));case 3:return function(r,s){if(typeof r.timestampValue=="string"&&typeof s.timestampValue=="string"&&r.timestampValue.length===s.timestampValue.length)return r.timestampValue===s.timestampValue;const o=Hi(r.timestampValue),a=Hi(s.timestampValue);return o.seconds===a.seconds&&o.nanos===a.nanos}(i,t);case 5:return i.stringValue===t.stringValue;case 6:return function(r,s){return pr(r.bytesValue).isEqual(pr(s.bytesValue))}(i,t);case 7:return i.referenceValue===t.referenceValue;case 8:return function(r,s){return Le(r.geoPointValue.latitude)===Le(s.geoPointValue.latitude)&&Le(r.geoPointValue.longitude)===Le(s.geoPointValue.longitude)}(i,t);case 2:return function(r,s){if("integerValue"in r&&"integerValue"in s)return Le(r.integerValue)===Le(s.integerValue);if("doubleValue"in r&&"doubleValue"in s){const o=Le(r.doubleValue),a=Le(s.doubleValue);return o===a?La(o)===La(a):isNaN(o)&&isNaN(a)}return!1}(i,t);case 9:return hs(i.arrayValue.values||[],t.arrayValue.values||[],Zn);case 10:case 11:return function(r,s){const o=r.mapValue.fields||{},a=s.mapValue.fields||{};if(tf(o)!==tf(a))return!1;for(const c in o)if(o.hasOwnProperty(c)&&(a[c]===void 0||!Zn(o[c],a[c])))return!1;return!0}(i,t);default:return $t()}}function lo(i,t){return(i.values||[]).find(e=>Zn(e,t))!==void 0}function us(i,t){if(i===t)return 0;const e=mr(i),n=mr(t);if(e!==n)return fe(e,n);switch(e){case 0:case 9007199254740991:return 0;case 1:return fe(i.booleanValue,t.booleanValue);case 2:return function(s,o){const a=Le(s.integerValue||s.doubleValue),c=Le(o.integerValue||o.doubleValue);return a<c?-1:a>c?1:a===c?0:isNaN(a)?isNaN(c)?0:-1:1}(i,t);case 3:return nf(i.timestampValue,t.timestampValue);case 4:return nf(ao(i),ao(t));case 5:return fe(i.stringValue,t.stringValue);case 6:return function(s,o){const a=pr(s),c=pr(o);return a.compareTo(c)}(i.bytesValue,t.bytesValue);case 7:return function(s,o){const a=s.split("/"),c=o.split("/");for(let h=0;h<a.length&&h<c.length;h++){const u=fe(a[h],c[h]);if(u!==0)return u}return fe(a.length,c.length)}(i.referenceValue,t.referenceValue);case 8:return function(s,o){const a=fe(Le(s.latitude),Le(o.latitude));return a!==0?a:fe(Le(s.longitude),Le(o.longitude))}(i.geoPointValue,t.geoPointValue);case 9:return rf(i.arrayValue,t.arrayValue);case 10:return function(s,o){var a,c,h,u;const d=s.fields||{},f=o.fields||{},p=(a=d.value)===null||a===void 0?void 0:a.arrayValue,y=(c=f.value)===null||c===void 0?void 0:c.arrayValue,E=fe(((h=p==null?void 0:p.values)===null||h===void 0?void 0:h.length)||0,((u=y==null?void 0:y.values)===null||u===void 0?void 0:u.length)||0);return E!==0?E:rf(p,y)}(i.mapValue,t.mapValue);case 11:return function(s,o){if(s===ha.mapValue&&o===ha.mapValue)return 0;if(s===ha.mapValue)return 1;if(o===ha.mapValue)return-1;const a=s.fields||{},c=Object.keys(a),h=o.fields||{},u=Object.keys(h);c.sort(),u.sort();for(let d=0;d<c.length&&d<u.length;++d){const f=fe(c[d],u[d]);if(f!==0)return f;const p=us(a[c[d]],h[u[d]]);if(p!==0)return p}return fe(c.length,u.length)}(i.mapValue,t.mapValue);default:throw $t()}}function nf(i,t){if(typeof i=="string"&&typeof t=="string"&&i.length===t.length)return fe(i,t);const e=Hi(i),n=Hi(t),r=fe(e.seconds,n.seconds);return r!==0?r:fe(e.nanos,n.nanos)}function rf(i,t){const e=i.values||[],n=t.values||[];for(let r=0;r<e.length&&r<n.length;++r){const s=us(e[r],n[r]);if(s)return s}return fe(e.length,n.length)}function ds(i){return Tl(i)}function Tl(i){return"nullValue"in i?"null":"booleanValue"in i?""+i.booleanValue:"integerValue"in i?""+i.integerValue:"doubleValue"in i?""+i.doubleValue:"timestampValue"in i?function(e){const n=Hi(e);return`time(${n.seconds},${n.nanos})`}(i.timestampValue):"stringValue"in i?i.stringValue:"bytesValue"in i?function(e){return pr(e).toBase64()}(i.bytesValue):"referenceValue"in i?function(e){return Bt.fromName(e).toString()}(i.referenceValue):"geoPointValue"in i?function(e){return`geo(${e.latitude},${e.longitude})`}(i.geoPointValue):"arrayValue"in i?function(e){let n="[",r=!0;for(const s of e.values||[])r?r=!1:n+=",",n+=Tl(s);return n+"]"}(i.arrayValue):"mapValue"in i?function(e){const n=Object.keys(e.fields||{}).sort();let r="{",s=!0;for(const o of n)s?s=!1:r+=",",r+=`${o}:${Tl(e.fields[o])}`;return r+"}"}(i.mapValue):$t()}function sf(i,t){return{referenceValue:`projects/${i.projectId}/databases/${i.database}/documents/${t.path.canonicalString()}`}}function Ml(i){return!!i&&"integerValue"in i}function th(i){return!!i&&"arrayValue"in i}function of(i){return!!i&&"nullValue"in i}function af(i){return!!i&&"doubleValue"in i&&isNaN(Number(i.doubleValue))}function ga(i){return!!i&&"mapValue"in i}function dM(i){var t,e;return((e=(((t=i==null?void 0:i.mapValue)===null||t===void 0?void 0:t.fields)||{}).__type__)===null||e===void 0?void 0:e.stringValue)==="__vector__"}function Ks(i){if(i.geoPointValue)return{geoPointValue:Object.assign({},i.geoPointValue)};if(i.timestampValue&&typeof i.timestampValue=="object")return{timestampValue:Object.assign({},i.timestampValue)};if(i.mapValue){const t={mapValue:{fields:{}}};return xs(i.mapValue.fields,(e,n)=>t.mapValue.fields[e]=Ks(n)),t}if(i.arrayValue){const t={arrayValue:{values:[]}};for(let e=0;e<(i.arrayValue.values||[]).length;++e)t.arrayValue.values[e]=Ks(i.arrayValue.values[e]);return t}return Object.assign({},i)}function fM(i){return(((i.mapValue||{}).fields||{}).__type__||{}).stringValue==="__max__"}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class Un{constructor(t){this.value=t}static empty(){return new Un({mapValue:{}})}field(t){if(t.isEmpty())return this.value;{let e=this.value;for(let n=0;n<t.length-1;++n)if(e=(e.mapValue.fields||{})[t.get(n)],!ga(e))return null;return e=(e.mapValue.fields||{})[t.lastSegment()],e||null}}set(t,e){this.getFieldsMap(t.popLast())[t.lastSegment()]=Ks(e)}setAll(t){let e=Je.emptyPath(),n={},r=[];t.forEach((o,a)=>{if(!e.isImmediateParentOf(a)){const c=this.getFieldsMap(e);this.applyChanges(c,n,r),n={},r=[],e=a.popLast()}o?n[a.lastSegment()]=Ks(o):r.push(a.lastSegment())});const s=this.getFieldsMap(e);this.applyChanges(s,n,r)}delete(t){const e=this.field(t.popLast());ga(e)&&e.mapValue.fields&&delete e.mapValue.fields[t.lastSegment()]}isEqual(t){return Zn(this.value,t.value)}getFieldsMap(t){let e=this.value;e.mapValue.fields||(e.mapValue={fields:{}});for(let n=0;n<t.length;++n){let r=e.mapValue.fields[t.get(n)];ga(r)&&r.mapValue.fields||(r={mapValue:{fields:{}}},e.mapValue.fields[t.get(n)]=r),e=r}return e.mapValue.fields}applyChanges(t,e,n){xs(e,(r,s)=>t[r]=s);for(const r of n)delete t[r]}clone(){return new Un(Ks(this.value))}}function Qp(i){const t=[];return xs(i.fields,(e,n)=>{const r=new Je([e]);if(ga(n)){const s=Qp(n.mapValue).fields;if(s.length===0)t.push(r);else for(const o of s)t.push(r.child(o))}else t.push(r)}),new qn(t)}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class un{constructor(t,e,n,r,s,o,a){this.key=t,this.documentType=e,this.version=n,this.readTime=r,this.createTime=s,this.data=o,this.documentState=a}static newInvalidDocument(t){return new un(t,0,Zt.min(),Zt.min(),Zt.min(),Un.empty(),0)}static newFoundDocument(t,e,n,r){return new un(t,1,e,Zt.min(),n,r,0)}static newNoDocument(t,e){return new un(t,2,e,Zt.min(),Zt.min(),Un.empty(),0)}static newUnknownDocument(t,e){return new un(t,3,e,Zt.min(),Zt.min(),Un.empty(),2)}convertToFoundDocument(t,e){return!this.createTime.isEqual(Zt.min())||this.documentType!==2&&this.documentType!==0||(this.createTime=t),this.version=t,this.documentType=1,this.data=e,this.documentState=0,this}convertToNoDocument(t){return this.version=t,this.documentType=2,this.data=Un.empty(),this.documentState=0,this}convertToUnknownDocument(t){return this.version=t,this.documentType=3,this.data=Un.empty(),this.documentState=2,this}setHasCommittedMutations(){return this.documentState=2,this}setHasLocalMutations(){return this.documentState=1,this.version=Zt.min(),this}setReadTime(t){return this.readTime=t,this}get hasLocalMutations(){return this.documentState===1}get hasCommittedMutations(){return this.documentState===2}get hasPendingWrites(){return this.hasLocalMutations||this.hasCommittedMutations}isValidDocument(){return this.documentType!==0}isFoundDocument(){return this.documentType===1}isNoDocument(){return this.documentType===2}isUnknownDocument(){return this.documentType===3}isEqual(t){return t instanceof un&&this.key.isEqual(t.key)&&this.version.isEqual(t.version)&&this.documentType===t.documentType&&this.documentState===t.documentState&&this.data.isEqual(t.data)}mutableCopy(){return new un(this.key,this.documentType,this.version,this.readTime,this.createTime,this.data.clone(),this.documentState)}toString(){return`Document(${this.key}, ${this.version}, ${JSON.stringify(this.data.value)}, {createTime: ${this.createTime}}), {documentType: ${this.documentType}}), {documentState: ${this.documentState}})`}}/**
 * @license
 * Copyright 2022 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class Na{constructor(t,e){this.position=t,this.inclusive=e}}function cf(i,t,e){let n=0;for(let r=0;r<i.position.length;r++){const s=t[r],o=i.position[r];if(s.field.isKeyField()?n=Bt.comparator(Bt.fromName(o.referenceValue),e.key):n=us(o,e.data.field(s.field)),s.dir==="desc"&&(n*=-1),n!==0)break}return n}function lf(i,t){if(i===null)return t===null;if(t===null||i.inclusive!==t.inclusive||i.position.length!==t.position.length)return!1;for(let e=0;e<i.position.length;e++)if(!Zn(i.position[e],t.position[e]))return!1;return!0}/**
 * @license
 * Copyright 2022 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class ho{constructor(t,e="asc"){this.field=t,this.dir=e}}function pM(i,t){return i.dir===t.dir&&i.field.isEqual(t.field)}/**
 * @license
 * Copyright 2022 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class Zp{}class Ve extends Zp{constructor(t,e,n){super(),this.field=t,this.op=e,this.value=n}static create(t,e,n){return t.isKeyField()?e==="in"||e==="not-in"?this.createKeyFieldInFilter(t,e,n):new gM(t,e,n):e==="array-contains"?new yM(t,n):e==="in"?new EM(t,n):e==="not-in"?new xM(t,n):e==="array-contains-any"?new SM(t,n):new Ve(t,e,n)}static createKeyFieldInFilter(t,e,n){return e==="in"?new _M(t,n):new vM(t,n)}matches(t){const e=t.data.field(this.field);return this.op==="!="?e!==null&&this.matchesComparison(us(e,this.value)):e!==null&&mr(this.value)===mr(e)&&this.matchesComparison(us(e,this.value))}matchesComparison(t){switch(this.op){case"<":return t<0;case"<=":return t<=0;case"==":return t===0;case"!=":return t!==0;case">":return t>0;case">=":return t>=0;default:return $t()}}isInequality(){return["<","<=",">",">=","!=","not-in"].indexOf(this.op)>=0}getFlattenedFilters(){return[this]}getFilters(){return[this]}}class Xn extends Zp{constructor(t,e){super(),this.filters=t,this.op=e,this.ae=null}static create(t,e){return new Xn(t,e)}matches(t){return Jp(this)?this.filters.find(e=>!e.matches(t))===void 0:this.filters.find(e=>e.matches(t))!==void 0}getFlattenedFilters(){return this.ae!==null||(this.ae=this.filters.reduce((t,e)=>t.concat(e.getFlattenedFilters()),[])),this.ae}getFilters(){return Object.assign([],this.filters)}}function Jp(i){return i.op==="and"}function tm(i){return mM(i)&&Jp(i)}function mM(i){for(const t of i.filters)if(t instanceof Xn)return!1;return!0}function wl(i){if(i instanceof Ve)return i.field.canonicalString()+i.op.toString()+ds(i.value);if(tm(i))return i.filters.map(t=>wl(t)).join(",");{const t=i.filters.map(e=>wl(e)).join(",");return`${i.op}(${t})`}}function em(i,t){return i instanceof Ve?function(n,r){return r instanceof Ve&&n.op===r.op&&n.field.isEqual(r.field)&&Zn(n.value,r.value)}(i,t):i instanceof Xn?function(n,r){return r instanceof Xn&&n.op===r.op&&n.filters.length===r.filters.length?n.filters.reduce((s,o,a)=>s&&em(o,r.filters[a]),!0):!1}(i,t):void $t()}function nm(i){return i instanceof Ve?function(e){return`${e.field.canonicalString()} ${e.op} ${ds(e.value)}`}(i):i instanceof Xn?function(e){return e.op.toString()+" {"+e.getFilters().map(nm).join(" ,")+"}"}(i):"Filter"}class gM extends Ve{constructor(t,e,n){super(t,e,n),this.key=Bt.fromName(n.referenceValue)}matches(t){const e=Bt.comparator(t.key,this.key);return this.matchesComparison(e)}}class _M extends Ve{constructor(t,e){super(t,"in",e),this.keys=im("in",e)}matches(t){return this.keys.some(e=>e.isEqual(t.key))}}class vM extends Ve{constructor(t,e){super(t,"not-in",e),this.keys=im("not-in",e)}matches(t){return!this.keys.some(e=>e.isEqual(t.key))}}function im(i,t){var e;return(((e=t.arrayValue)===null||e===void 0?void 0:e.values)||[]).map(n=>Bt.fromName(n.referenceValue))}class yM extends Ve{constructor(t,e){super(t,"array-contains",e)}matches(t){const e=t.data.field(this.field);return th(e)&&lo(e.arrayValue,this.value)}}class EM extends Ve{constructor(t,e){super(t,"in",e)}matches(t){const e=t.data.field(this.field);return e!==null&&lo(this.value.arrayValue,e)}}class xM extends Ve{constructor(t,e){super(t,"not-in",e)}matches(t){if(lo(this.value.arrayValue,{nullValue:"NULL_VALUE"}))return!1;const e=t.data.field(this.field);return e!==null&&!lo(this.value.arrayValue,e)}}class SM extends Ve{constructor(t,e){super(t,"array-contains-any",e)}matches(t){const e=t.data.field(this.field);return!(!th(e)||!e.arrayValue.values)&&e.arrayValue.values.some(n=>lo(this.value.arrayValue,n))}}/**
 * @license
 * Copyright 2019 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class TM{constructor(t,e=null,n=[],r=[],s=null,o=null,a=null){this.path=t,this.collectionGroup=e,this.orderBy=n,this.filters=r,this.limit=s,this.startAt=o,this.endAt=a,this.ue=null}}function hf(i,t=null,e=[],n=[],r=null,s=null,o=null){return new TM(i,t,e,n,r,s,o)}function eh(i){const t=Jt(i);if(t.ue===null){let e=t.path.canonicalString();t.collectionGroup!==null&&(e+="|cg:"+t.collectionGroup),e+="|f:",e+=t.filters.map(n=>wl(n)).join(","),e+="|ob:",e+=t.orderBy.map(n=>function(s){return s.field.canonicalString()+s.dir}(n)).join(","),$a(t.limit)||(e+="|l:",e+=t.limit),t.startAt&&(e+="|lb:",e+=t.startAt.inclusive?"b:":"a:",e+=t.startAt.position.map(n=>ds(n)).join(",")),t.endAt&&(e+="|ub:",e+=t.endAt.inclusive?"a:":"b:",e+=t.endAt.position.map(n=>ds(n)).join(",")),t.ue=e}return t.ue}function nh(i,t){if(i.limit!==t.limit||i.orderBy.length!==t.orderBy.length)return!1;for(let e=0;e<i.orderBy.length;e++)if(!pM(i.orderBy[e],t.orderBy[e]))return!1;if(i.filters.length!==t.filters.length)return!1;for(let e=0;e<i.filters.length;e++)if(!em(i.filters[e],t.filters[e]))return!1;return i.collectionGroup===t.collectionGroup&&!!i.path.isEqual(t.path)&&!!lf(i.startAt,t.startAt)&&lf(i.endAt,t.endAt)}function bl(i){return Bt.isDocumentKey(i.path)&&i.collectionGroup===null&&i.filters.length===0}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class Ss{constructor(t,e=null,n=[],r=[],s=null,o="F",a=null,c=null){this.path=t,this.collectionGroup=e,this.explicitOrderBy=n,this.filters=r,this.limit=s,this.limitType=o,this.startAt=a,this.endAt=c,this.ce=null,this.le=null,this.he=null,this.startAt,this.endAt}}function MM(i,t,e,n,r,s,o,a){return new Ss(i,t,e,n,r,s,o,a)}function rm(i){return new Ss(i)}function uf(i){return i.filters.length===0&&i.limit===null&&i.startAt==null&&i.endAt==null&&(i.explicitOrderBy.length===0||i.explicitOrderBy.length===1&&i.explicitOrderBy[0].field.isKeyField())}function sm(i){return i.collectionGroup!==null}function Qs(i){const t=Jt(i);if(t.ce===null){t.ce=[];const e=new Set;for(const s of t.explicitOrderBy)t.ce.push(s),e.add(s.field.canonicalString());const n=t.explicitOrderBy.length>0?t.explicitOrderBy[t.explicitOrderBy.length-1].dir:"asc";(function(o){let a=new tn(Je.comparator);return o.filters.forEach(c=>{c.getFlattenedFilters().forEach(h=>{h.isInequality()&&(a=a.add(h.field))})}),a})(t).forEach(s=>{e.has(s.canonicalString())||s.isKeyField()||t.ce.push(new ho(s,n))}),e.has(Je.keyField().canonicalString())||t.ce.push(new ho(Je.keyField(),n))}return t.ce}function Kn(i){const t=Jt(i);return t.le||(t.le=wM(t,Qs(i))),t.le}function wM(i,t){if(i.limitType==="F")return hf(i.path,i.collectionGroup,t,i.filters,i.limit,i.startAt,i.endAt);{t=t.map(r=>{const s=r.dir==="desc"?"asc":"desc";return new ho(r.field,s)});const e=i.endAt?new Na(i.endAt.position,i.endAt.inclusive):null,n=i.startAt?new Na(i.startAt.position,i.startAt.inclusive):null;return hf(i.path,i.collectionGroup,t,i.filters,i.limit,e,n)}}function Al(i,t){const e=i.filters.concat([t]);return new Ss(i.path,i.collectionGroup,i.explicitOrderBy.slice(),e,i.limit,i.limitType,i.startAt,i.endAt)}function Ua(i,t,e){return new Ss(i.path,i.collectionGroup,i.explicitOrderBy.slice(),i.filters.slice(),t,e,i.startAt,i.endAt)}function Ka(i,t){return nh(Kn(i),Kn(t))&&i.limitType===t.limitType}function om(i){return`${eh(Kn(i))}|lt:${i.limitType}`}function Yr(i){return`Query(target=${function(e){let n=e.path.canonicalString();return e.collectionGroup!==null&&(n+=" collectionGroup="+e.collectionGroup),e.filters.length>0&&(n+=`, filters: [${e.filters.map(r=>nm(r)).join(", ")}]`),$a(e.limit)||(n+=", limit: "+e.limit),e.orderBy.length>0&&(n+=`, orderBy: [${e.orderBy.map(r=>function(o){return`${o.field.canonicalString()} (${o.dir})`}(r)).join(", ")}]`),e.startAt&&(n+=", startAt: ",n+=e.startAt.inclusive?"b:":"a:",n+=e.startAt.position.map(r=>ds(r)).join(",")),e.endAt&&(n+=", endAt: ",n+=e.endAt.inclusive?"a:":"b:",n+=e.endAt.position.map(r=>ds(r)).join(",")),`Target(${n})`}(Kn(i))}; limitType=${i.limitType})`}function Qa(i,t){return t.isFoundDocument()&&function(n,r){const s=r.key.path;return n.collectionGroup!==null?r.key.hasCollectionId(n.collectionGroup)&&n.path.isPrefixOf(s):Bt.isDocumentKey(n.path)?n.path.isEqual(s):n.path.isImmediateParentOf(s)}(i,t)&&function(n,r){for(const s of Qs(n))if(!s.field.isKeyField()&&r.data.field(s.field)===null)return!1;return!0}(i,t)&&function(n,r){for(const s of n.filters)if(!s.matches(r))return!1;return!0}(i,t)&&function(n,r){return!(n.startAt&&!function(o,a,c){const h=cf(o,a,c);return o.inclusive?h<=0:h<0}(n.startAt,Qs(n),r)||n.endAt&&!function(o,a,c){const h=cf(o,a,c);return o.inclusive?h>=0:h>0}(n.endAt,Qs(n),r))}(i,t)}function bM(i){return i.collectionGroup||(i.path.length%2==1?i.path.lastSegment():i.path.get(i.path.length-2))}function am(i){return(t,e)=>{let n=!1;for(const r of Qs(i)){const s=AM(r,t,e);if(s!==0)return s;n=n||r.field.isKeyField()}return 0}}function AM(i,t,e){const n=i.field.isKeyField()?Bt.comparator(t.key,e.key):function(s,o,a){const c=o.data.field(s),h=a.data.field(s);return c!==null&&h!==null?us(c,h):$t()}(i.field,t,e);switch(i.dir){case"asc":return n;case"desc":return-1*n;default:return $t()}}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class Ts{constructor(t,e){this.mapKeyFn=t,this.equalsFn=e,this.inner={},this.innerSize=0}get(t){const e=this.mapKeyFn(t),n=this.inner[e];if(n!==void 0){for(const[r,s]of n)if(this.equalsFn(r,t))return s}}has(t){return this.get(t)!==void 0}set(t,e){const n=this.mapKeyFn(t),r=this.inner[n];if(r===void 0)return this.inner[n]=[[t,e]],void this.innerSize++;for(let s=0;s<r.length;s++)if(this.equalsFn(r[s][0],t))return void(r[s]=[t,e]);r.push([t,e]),this.innerSize++}delete(t){const e=this.mapKeyFn(t),n=this.inner[e];if(n===void 0)return!1;for(let r=0;r<n.length;r++)if(this.equalsFn(n[r][0],t))return n.length===1?delete this.inner[e]:n.splice(r,1),this.innerSize--,!0;return!1}forEach(t){xs(this.inner,(e,n)=>{for(const[r,s]of n)t(r,s)})}isEmpty(){return $p(this.inner)}size(){return this.innerSize}}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */const RM=new Pe(Bt.comparator);function gi(){return RM}const cm=new Pe(Bt.comparator);function qs(...i){let t=cm;for(const e of i)t=t.insert(e.key,e);return t}function lm(i){let t=cm;return i.forEach((e,n)=>t=t.insert(e,n.overlayedDocument)),t}function sr(){return Zs()}function hm(){return Zs()}function Zs(){return new Ts(i=>i.toString(),(i,t)=>i.isEqual(t))}const CM=new Pe(Bt.comparator),IM=new tn(Bt.comparator);function ce(...i){let t=IM;for(const e of i)t=t.add(e);return t}const PM=new tn(fe);function DM(){return PM}/**
 * @license
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */function ih(i,t){if(i.useProto3Json){if(isNaN(t))return{doubleValue:"NaN"};if(t===1/0)return{doubleValue:"Infinity"};if(t===-1/0)return{doubleValue:"-Infinity"}}return{doubleValue:La(t)?"-0":t}}function um(i){return{integerValue:""+i}}function LM(i,t){return lM(t)?um(t):ih(i,t)}/**
 * @license
 * Copyright 2018 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class Za{constructor(){this._=void 0}}function NM(i,t,e){return i instanceof Oa?function(r,s){const o={fields:{__type__:{stringValue:"server_timestamp"},__local_write_time__:{timestampValue:{seconds:r.seconds,nanos:r.nanoseconds}}}};return s&&Zl(s)&&(s=Jl(s)),s&&(o.fields.__previous_value__=s),{mapValue:o}}(e,t):i instanceof uo?fm(i,t):i instanceof fo?pm(i,t):function(r,s){const o=dm(r,s),a=df(o)+df(r.Pe);return Ml(o)&&Ml(r.Pe)?um(a):ih(r.serializer,a)}(i,t)}function UM(i,t,e){return i instanceof uo?fm(i,t):i instanceof fo?pm(i,t):e}function dm(i,t){return i instanceof Fa?function(n){return Ml(n)||function(s){return!!s&&"doubleValue"in s}(n)}(t)?t:{integerValue:0}:null}class Oa extends Za{}class uo extends Za{constructor(t){super(),this.elements=t}}function fm(i,t){const e=mm(t);for(const n of i.elements)e.some(r=>Zn(r,n))||e.push(n);return{arrayValue:{values:e}}}class fo extends Za{constructor(t){super(),this.elements=t}}function pm(i,t){let e=mm(t);for(const n of i.elements)e=e.filter(r=>!Zn(r,n));return{arrayValue:{values:e}}}class Fa extends Za{constructor(t,e){super(),this.serializer=t,this.Pe=e}}function df(i){return Le(i.integerValue||i.doubleValue)}function mm(i){return th(i)&&i.arrayValue.values?i.arrayValue.values.slice():[]}function OM(i,t){return i.field.isEqual(t.field)&&function(n,r){return n instanceof uo&&r instanceof uo||n instanceof fo&&r instanceof fo?hs(n.elements,r.elements,Zn):n instanceof Fa&&r instanceof Fa?Zn(n.Pe,r.Pe):n instanceof Oa&&r instanceof Oa}(i.transform,t.transform)}class FM{constructor(t,e){this.version=t,this.transformResults=e}}class hi{constructor(t,e){this.updateTime=t,this.exists=e}static none(){return new hi}static exists(t){return new hi(void 0,t)}static updateTime(t){return new hi(t)}get isNone(){return this.updateTime===void 0&&this.exists===void 0}isEqual(t){return this.exists===t.exists&&(this.updateTime?!!t.updateTime&&this.updateTime.isEqual(t.updateTime):!t.updateTime)}}function _a(i,t){return i.updateTime!==void 0?t.isFoundDocument()&&t.version.isEqual(i.updateTime):i.exists===void 0||i.exists===t.isFoundDocument()}class Ja{}function gm(i,t){if(!i.hasLocalMutations||t&&t.fields.length===0)return null;if(t===null)return i.isNoDocument()?new vm(i.key,hi.none()):new vo(i.key,i.data,hi.none());{const e=i.data,n=Un.empty();let r=new tn(Je.comparator);for(let s of t.fields)if(!r.has(s)){let o=e.field(s);o===null&&s.length>1&&(s=s.popLast(),o=e.field(s)),o===null?n.delete(s):n.set(s,o),r=r.add(s)}return new _r(i.key,n,new qn(r.toArray()),hi.none())}}function VM(i,t,e){i instanceof vo?function(r,s,o){const a=r.value.clone(),c=pf(r.fieldTransforms,s,o.transformResults);a.setAll(c),s.convertToFoundDocument(o.version,a).setHasCommittedMutations()}(i,t,e):i instanceof _r?function(r,s,o){if(!_a(r.precondition,s))return void s.convertToUnknownDocument(o.version);const a=pf(r.fieldTransforms,s,o.transformResults),c=s.data;c.setAll(_m(r)),c.setAll(a),s.convertToFoundDocument(o.version,c).setHasCommittedMutations()}(i,t,e):function(r,s,o){s.convertToNoDocument(o.version).setHasCommittedMutations()}(0,t,e)}function Js(i,t,e,n){return i instanceof vo?function(s,o,a,c){if(!_a(s.precondition,o))return a;const h=s.value.clone(),u=mf(s.fieldTransforms,c,o);return h.setAll(u),o.convertToFoundDocument(o.version,h).setHasLocalMutations(),null}(i,t,e,n):i instanceof _r?function(s,o,a,c){if(!_a(s.precondition,o))return a;const h=mf(s.fieldTransforms,c,o),u=o.data;return u.setAll(_m(s)),u.setAll(h),o.convertToFoundDocument(o.version,u).setHasLocalMutations(),a===null?null:a.unionWith(s.fieldMask.fields).unionWith(s.fieldTransforms.map(d=>d.field))}(i,t,e,n):function(s,o,a){return _a(s.precondition,o)?(o.convertToNoDocument(o.version).setHasLocalMutations(),null):a}(i,t,e)}function BM(i,t){let e=null;for(const n of i.fieldTransforms){const r=t.data.field(n.field),s=dm(n.transform,r||null);s!=null&&(e===null&&(e=Un.empty()),e.set(n.field,s))}return e||null}function ff(i,t){return i.type===t.type&&!!i.key.isEqual(t.key)&&!!i.precondition.isEqual(t.precondition)&&!!function(n,r){return n===void 0&&r===void 0||!(!n||!r)&&hs(n,r,(s,o)=>OM(s,o))}(i.fieldTransforms,t.fieldTransforms)&&(i.type===0?i.value.isEqual(t.value):i.type!==1||i.data.isEqual(t.data)&&i.fieldMask.isEqual(t.fieldMask))}class vo extends Ja{constructor(t,e,n,r=[]){super(),this.key=t,this.value=e,this.precondition=n,this.fieldTransforms=r,this.type=0}getFieldMask(){return null}}class _r extends Ja{constructor(t,e,n,r,s=[]){super(),this.key=t,this.data=e,this.fieldMask=n,this.precondition=r,this.fieldTransforms=s,this.type=1}getFieldMask(){return this.fieldMask}}function _m(i){const t=new Map;return i.fieldMask.fields.forEach(e=>{if(!e.isEmpty()){const n=i.data.field(e);t.set(e,n)}}),t}function pf(i,t,e){const n=new Map;ge(i.length===e.length);for(let r=0;r<e.length;r++){const s=i[r],o=s.transform,a=t.data.field(s.field);n.set(s.field,UM(o,a,e[r]))}return n}function mf(i,t,e){const n=new Map;for(const r of i){const s=r.transform,o=e.data.field(r.field);n.set(r.field,NM(s,o,t))}return n}class vm extends Ja{constructor(t,e){super(),this.key=t,this.precondition=e,this.type=2,this.fieldTransforms=[]}getFieldMask(){return null}}class kM extends Ja{constructor(t,e){super(),this.key=t,this.precondition=e,this.type=3,this.fieldTransforms=[]}getFieldMask(){return null}}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class HM{constructor(t,e,n,r){this.batchId=t,this.localWriteTime=e,this.baseMutations=n,this.mutations=r}applyToRemoteDocument(t,e){const n=e.mutationResults;for(let r=0;r<this.mutations.length;r++){const s=this.mutations[r];s.key.isEqual(t.key)&&VM(s,t,n[r])}}applyToLocalView(t,e){for(const n of this.baseMutations)n.key.isEqual(t.key)&&(e=Js(n,t,e,this.localWriteTime));for(const n of this.mutations)n.key.isEqual(t.key)&&(e=Js(n,t,e,this.localWriteTime));return e}applyToLocalDocumentSet(t,e){const n=hm();return this.mutations.forEach(r=>{const s=t.get(r.key),o=s.overlayedDocument;let a=this.applyToLocalView(o,s.mutatedFields);a=e.has(r.key)?null:a;const c=gm(o,a);c!==null&&n.set(r.key,c),o.isValidDocument()||o.convertToNoDocument(Zt.min())}),n}keys(){return this.mutations.reduce((t,e)=>t.add(e.key),ce())}isEqual(t){return this.batchId===t.batchId&&hs(this.mutations,t.mutations,(e,n)=>ff(e,n))&&hs(this.baseMutations,t.baseMutations,(e,n)=>ff(e,n))}}class rh{constructor(t,e,n,r){this.batch=t,this.commitVersion=e,this.mutationResults=n,this.docVersions=r}static from(t,e,n){ge(t.mutations.length===n.length);let r=function(){return CM}();const s=t.mutations;for(let o=0;o<s.length;o++)r=r.insert(s[o].key,n[o].version);return new rh(t,e,n,r)}}/**
 * @license
 * Copyright 2022 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class zM{constructor(t,e){this.largestBatchId=t,this.mutation=e}getKey(){return this.mutation.key}isEqual(t){return t!==null&&this.mutation===t.mutation}toString(){return`Overlay{
      largestBatchId: ${this.largestBatchId},
      mutation: ${this.mutation.toString()}
    }`}}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class GM{constructor(t,e){this.count=t,this.unchangedNames=e}}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */var Fe,le;function WM(i){switch(i){default:return $t();case nt.CANCELLED:case nt.UNKNOWN:case nt.DEADLINE_EXCEEDED:case nt.RESOURCE_EXHAUSTED:case nt.INTERNAL:case nt.UNAVAILABLE:case nt.UNAUTHENTICATED:return!1;case nt.INVALID_ARGUMENT:case nt.NOT_FOUND:case nt.ALREADY_EXISTS:case nt.PERMISSION_DENIED:case nt.FAILED_PRECONDITION:case nt.ABORTED:case nt.OUT_OF_RANGE:case nt.UNIMPLEMENTED:case nt.DATA_LOSS:return!0}}function ym(i){if(i===void 0)return mi("GRPC error has no .code"),nt.UNKNOWN;switch(i){case Fe.OK:return nt.OK;case Fe.CANCELLED:return nt.CANCELLED;case Fe.UNKNOWN:return nt.UNKNOWN;case Fe.DEADLINE_EXCEEDED:return nt.DEADLINE_EXCEEDED;case Fe.RESOURCE_EXHAUSTED:return nt.RESOURCE_EXHAUSTED;case Fe.INTERNAL:return nt.INTERNAL;case Fe.UNAVAILABLE:return nt.UNAVAILABLE;case Fe.UNAUTHENTICATED:return nt.UNAUTHENTICATED;case Fe.INVALID_ARGUMENT:return nt.INVALID_ARGUMENT;case Fe.NOT_FOUND:return nt.NOT_FOUND;case Fe.ALREADY_EXISTS:return nt.ALREADY_EXISTS;case Fe.PERMISSION_DENIED:return nt.PERMISSION_DENIED;case Fe.FAILED_PRECONDITION:return nt.FAILED_PRECONDITION;case Fe.ABORTED:return nt.ABORTED;case Fe.OUT_OF_RANGE:return nt.OUT_OF_RANGE;case Fe.UNIMPLEMENTED:return nt.UNIMPLEMENTED;case Fe.DATA_LOSS:return nt.DATA_LOSS;default:return $t()}}(le=Fe||(Fe={}))[le.OK=0]="OK",le[le.CANCELLED=1]="CANCELLED",le[le.UNKNOWN=2]="UNKNOWN",le[le.INVALID_ARGUMENT=3]="INVALID_ARGUMENT",le[le.DEADLINE_EXCEEDED=4]="DEADLINE_EXCEEDED",le[le.NOT_FOUND=5]="NOT_FOUND",le[le.ALREADY_EXISTS=6]="ALREADY_EXISTS",le[le.PERMISSION_DENIED=7]="PERMISSION_DENIED",le[le.UNAUTHENTICATED=16]="UNAUTHENTICATED",le[le.RESOURCE_EXHAUSTED=8]="RESOURCE_EXHAUSTED",le[le.FAILED_PRECONDITION=9]="FAILED_PRECONDITION",le[le.ABORTED=10]="ABORTED",le[le.OUT_OF_RANGE=11]="OUT_OF_RANGE",le[le.UNIMPLEMENTED=12]="UNIMPLEMENTED",le[le.INTERNAL=13]="INTERNAL",le[le.UNAVAILABLE=14]="UNAVAILABLE",le[le.DATA_LOSS=15]="DATA_LOSS";/**
 * @license
 * Copyright 2023 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */function qM(){return new TextEncoder}/**
 * @license
 * Copyright 2022 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */const XM=new lr([4294967295,4294967295],0);function gf(i){const t=qM().encode(i),e=new Hp;return e.update(t),new Uint8Array(e.digest())}function _f(i){const t=new DataView(i.buffer),e=t.getUint32(0,!0),n=t.getUint32(4,!0),r=t.getUint32(8,!0),s=t.getUint32(12,!0);return[new lr([e,n],0),new lr([r,s],0)]}class sh{constructor(t,e,n){if(this.bitmap=t,this.padding=e,this.hashCount=n,e<0||e>=8)throw new Xs(`Invalid padding: ${e}`);if(n<0)throw new Xs(`Invalid hash count: ${n}`);if(t.length>0&&this.hashCount===0)throw new Xs(`Invalid hash count: ${n}`);if(t.length===0&&e!==0)throw new Xs(`Invalid padding when bitmap length is 0: ${e}`);this.Ie=8*t.length-e,this.Te=lr.fromNumber(this.Ie)}Ee(t,e,n){let r=t.add(e.multiply(lr.fromNumber(n)));return r.compare(XM)===1&&(r=new lr([r.getBits(0),r.getBits(1)],0)),r.modulo(this.Te).toNumber()}de(t){return(this.bitmap[Math.floor(t/8)]&1<<t%8)!=0}mightContain(t){if(this.Ie===0)return!1;const e=gf(t),[n,r]=_f(e);for(let s=0;s<this.hashCount;s++){const o=this.Ee(n,r,s);if(!this.de(o))return!1}return!0}static create(t,e,n){const r=t%8==0?0:8-t%8,s=new Uint8Array(Math.ceil(t/8)),o=new sh(s,r,e);return n.forEach(a=>o.insert(a)),o}insert(t){if(this.Ie===0)return;const e=gf(t),[n,r]=_f(e);for(let s=0;s<this.hashCount;s++){const o=this.Ee(n,r,s);this.Ae(o)}}Ae(t){const e=Math.floor(t/8),n=t%8;this.bitmap[e]|=1<<n}}class Xs extends Error{constructor(){super(...arguments),this.name="BloomFilterError"}}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class tc{constructor(t,e,n,r,s){this.snapshotVersion=t,this.targetChanges=e,this.targetMismatches=n,this.documentUpdates=r,this.resolvedLimboDocuments=s}static createSynthesizedRemoteEventForCurrentChange(t,e,n){const r=new Map;return r.set(t,yo.createSynthesizedTargetChangeForCurrentChange(t,e,n)),new tc(Zt.min(),r,new Pe(fe),gi(),ce())}}class yo{constructor(t,e,n,r,s){this.resumeToken=t,this.current=e,this.addedDocuments=n,this.modifiedDocuments=r,this.removedDocuments=s}static createSynthesizedTargetChangeForCurrentChange(t,e,n){return new yo(n,e,ce(),ce(),ce())}}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class va{constructor(t,e,n,r){this.Re=t,this.removedTargetIds=e,this.key=n,this.Ve=r}}class Em{constructor(t,e){this.targetId=t,this.me=e}}class xm{constructor(t,e,n=en.EMPTY_BYTE_STRING,r=null){this.state=t,this.targetIds=e,this.resumeToken=n,this.cause=r}}class vf{constructor(){this.fe=0,this.ge=Ef(),this.pe=en.EMPTY_BYTE_STRING,this.ye=!1,this.we=!0}get current(){return this.ye}get resumeToken(){return this.pe}get Se(){return this.fe!==0}get be(){return this.we}De(t){t.approximateByteSize()>0&&(this.we=!0,this.pe=t)}ve(){let t=ce(),e=ce(),n=ce();return this.ge.forEach((r,s)=>{switch(s){case 0:t=t.add(r);break;case 2:e=e.add(r);break;case 1:n=n.add(r);break;default:$t()}}),new yo(this.pe,this.ye,t,e,n)}Ce(){this.we=!1,this.ge=Ef()}Fe(t,e){this.we=!0,this.ge=this.ge.insert(t,e)}Me(t){this.we=!0,this.ge=this.ge.remove(t)}xe(){this.fe+=1}Oe(){this.fe-=1,ge(this.fe>=0)}Ne(){this.we=!0,this.ye=!0}}class jM{constructor(t){this.Le=t,this.Be=new Map,this.ke=gi(),this.qe=yf(),this.Qe=new Pe(fe)}Ke(t){for(const e of t.Re)t.Ve&&t.Ve.isFoundDocument()?this.$e(e,t.Ve):this.Ue(e,t.key,t.Ve);for(const e of t.removedTargetIds)this.Ue(e,t.key,t.Ve)}We(t){this.forEachTarget(t,e=>{const n=this.Ge(e);switch(t.state){case 0:this.ze(e)&&n.De(t.resumeToken);break;case 1:n.Oe(),n.Se||n.Ce(),n.De(t.resumeToken);break;case 2:n.Oe(),n.Se||this.removeTarget(e);break;case 3:this.ze(e)&&(n.Ne(),n.De(t.resumeToken));break;case 4:this.ze(e)&&(this.je(e),n.De(t.resumeToken));break;default:$t()}})}forEachTarget(t,e){t.targetIds.length>0?t.targetIds.forEach(e):this.Be.forEach((n,r)=>{this.ze(r)&&e(r)})}He(t){const e=t.targetId,n=t.me.count,r=this.Je(e);if(r){const s=r.target;if(bl(s))if(n===0){const o=new Bt(s.path);this.Ue(e,o,un.newNoDocument(o,Zt.min()))}else ge(n===1);else{const o=this.Ye(e);if(o!==n){const a=this.Ze(t),c=a?this.Xe(a,t,o):1;if(c!==0){this.je(e);const h=c===2?"TargetPurposeExistenceFilterMismatchBloom":"TargetPurposeExistenceFilterMismatch";this.Qe=this.Qe.insert(e,h)}}}}}Ze(t){const e=t.me.unchangedNames;if(!e||!e.bits)return null;const{bits:{bitmap:n="",padding:r=0},hashCount:s=0}=e;let o,a;try{o=pr(n).toUint8Array()}catch(c){if(c instanceof Kp)return ls("Decoding the base64 bloom filter in existence filter failed ("+c.message+"); ignoring the bloom filter and falling back to full re-query."),null;throw c}try{a=new sh(o,r,s)}catch(c){return ls(c instanceof Xs?"BloomFilter error: ":"Applying bloom filter failed: ",c),null}return a.Ie===0?null:a}Xe(t,e,n){return e.me.count===n-this.nt(t,e.targetId)?0:2}nt(t,e){const n=this.Le.getRemoteKeysForTarget(e);let r=0;return n.forEach(s=>{const o=this.Le.tt(),a=`projects/${o.projectId}/databases/${o.database}/documents/${s.path.canonicalString()}`;t.mightContain(a)||(this.Ue(e,s,null),r++)}),r}rt(t){const e=new Map;this.Be.forEach((s,o)=>{const a=this.Je(o);if(a){if(s.current&&bl(a.target)){const c=new Bt(a.target.path);this.ke.get(c)!==null||this.it(o,c)||this.Ue(o,c,un.newNoDocument(c,t))}s.be&&(e.set(o,s.ve()),s.Ce())}});let n=ce();this.qe.forEach((s,o)=>{let a=!0;o.forEachWhile(c=>{const h=this.Je(c);return!h||h.purpose==="TargetPurposeLimboResolution"||(a=!1,!1)}),a&&(n=n.add(s))}),this.ke.forEach((s,o)=>o.setReadTime(t));const r=new tc(t,e,this.Qe,this.ke,n);return this.ke=gi(),this.qe=yf(),this.Qe=new Pe(fe),r}$e(t,e){if(!this.ze(t))return;const n=this.it(t,e.key)?2:0;this.Ge(t).Fe(e.key,n),this.ke=this.ke.insert(e.key,e),this.qe=this.qe.insert(e.key,this.st(e.key).add(t))}Ue(t,e,n){if(!this.ze(t))return;const r=this.Ge(t);this.it(t,e)?r.Fe(e,1):r.Me(e),this.qe=this.qe.insert(e,this.st(e).delete(t)),n&&(this.ke=this.ke.insert(e,n))}removeTarget(t){this.Be.delete(t)}Ye(t){const e=this.Ge(t).ve();return this.Le.getRemoteKeysForTarget(t).size+e.addedDocuments.size-e.removedDocuments.size}xe(t){this.Ge(t).xe()}Ge(t){let e=this.Be.get(t);return e||(e=new vf,this.Be.set(t,e)),e}st(t){let e=this.qe.get(t);return e||(e=new tn(fe),this.qe=this.qe.insert(t,e)),e}ze(t){const e=this.Je(t)!==null;return e||At("WatchChangeAggregator","Detected inactive target",t),e}Je(t){const e=this.Be.get(t);return e&&e.Se?null:this.Le.ot(t)}je(t){this.Be.set(t,new vf),this.Le.getRemoteKeysForTarget(t).forEach(e=>{this.Ue(t,e,null)})}it(t,e){return this.Le.getRemoteKeysForTarget(t).has(e)}}function yf(){return new Pe(Bt.comparator)}function Ef(){return new Pe(Bt.comparator)}const YM={asc:"ASCENDING",desc:"DESCENDING"},$M={"<":"LESS_THAN","<=":"LESS_THAN_OR_EQUAL",">":"GREATER_THAN",">=":"GREATER_THAN_OR_EQUAL","==":"EQUAL","!=":"NOT_EQUAL","array-contains":"ARRAY_CONTAINS",in:"IN","not-in":"NOT_IN","array-contains-any":"ARRAY_CONTAINS_ANY"},KM={and:"AND",or:"OR"};class QM{constructor(t,e){this.databaseId=t,this.useProto3Json=e}}function Rl(i,t){return i.useProto3Json||$a(t)?t:{value:t}}function Va(i,t){return i.useProto3Json?`${new Date(1e3*t.seconds).toISOString().replace(/\.\d*/,"").replace("Z","")}.${("000000000"+t.nanoseconds).slice(-9)}Z`:{seconds:""+t.seconds,nanos:t.nanoseconds}}function Sm(i,t){return i.useProto3Json?t.toBase64():t.toUint8Array()}function ZM(i,t){return Va(i,t.toTimestamp())}function Qn(i){return ge(!!i),Zt.fromTimestamp(function(e){const n=Hi(e);return new He(n.seconds,n.nanos)}(i))}function oh(i,t){return Cl(i,t).canonicalString()}function Cl(i,t){const e=function(r){return new be(["projects",r.projectId,"databases",r.database])}(i).child("documents");return t===void 0?e:e.child(t)}function Tm(i){const t=be.fromString(i);return ge(Rm(t)),t}function Il(i,t){return oh(i.databaseId,t.path)}function Jc(i,t){const e=Tm(t);if(e.get(1)!==i.databaseId.projectId)throw new Tt(nt.INVALID_ARGUMENT,"Tried to deserialize key from different project: "+e.get(1)+" vs "+i.databaseId.projectId);if(e.get(3)!==i.databaseId.database)throw new Tt(nt.INVALID_ARGUMENT,"Tried to deserialize key from different database: "+e.get(3)+" vs "+i.databaseId.database);return new Bt(wm(e))}function Mm(i,t){return oh(i.databaseId,t)}function JM(i){const t=Tm(i);return t.length===4?be.emptyPath():wm(t)}function Pl(i){return new be(["projects",i.databaseId.projectId,"databases",i.databaseId.database]).canonicalString()}function wm(i){return ge(i.length>4&&i.get(4)==="documents"),i.popFirst(5)}function xf(i,t,e){return{name:Il(i,t),fields:e.value.mapValue.fields}}function tw(i,t){let e;if("targetChange"in t){t.targetChange;const n=function(h){return h==="NO_CHANGE"?0:h==="ADD"?1:h==="REMOVE"?2:h==="CURRENT"?3:h==="RESET"?4:$t()}(t.targetChange.targetChangeType||"NO_CHANGE"),r=t.targetChange.targetIds||[],s=function(h,u){return h.useProto3Json?(ge(u===void 0||typeof u=="string"),en.fromBase64String(u||"")):(ge(u===void 0||u instanceof Buffer||u instanceof Uint8Array),en.fromUint8Array(u||new Uint8Array))}(i,t.targetChange.resumeToken),o=t.targetChange.cause,a=o&&function(h){const u=h.code===void 0?nt.UNKNOWN:ym(h.code);return new Tt(u,h.message||"")}(o);e=new xm(n,r,s,a||null)}else if("documentChange"in t){t.documentChange;const n=t.documentChange;n.document,n.document.name,n.document.updateTime;const r=Jc(i,n.document.name),s=Qn(n.document.updateTime),o=n.document.createTime?Qn(n.document.createTime):Zt.min(),a=new Un({mapValue:{fields:n.document.fields}}),c=un.newFoundDocument(r,s,o,a),h=n.targetIds||[],u=n.removedTargetIds||[];e=new va(h,u,c.key,c)}else if("documentDelete"in t){t.documentDelete;const n=t.documentDelete;n.document;const r=Jc(i,n.document),s=n.readTime?Qn(n.readTime):Zt.min(),o=un.newNoDocument(r,s),a=n.removedTargetIds||[];e=new va([],a,o.key,o)}else if("documentRemove"in t){t.documentRemove;const n=t.documentRemove;n.document;const r=Jc(i,n.document),s=n.removedTargetIds||[];e=new va([],s,r,null)}else{if(!("filter"in t))return $t();{t.filter;const n=t.filter;n.targetId;const{count:r=0,unchangedNames:s}=n,o=new GM(r,s),a=n.targetId;e=new Em(a,o)}}return e}function ew(i,t){let e;if(t instanceof vo)e={update:xf(i,t.key,t.value)};else if(t instanceof vm)e={delete:Il(i,t.key)};else if(t instanceof _r)e={update:xf(i,t.key,t.data),updateMask:hw(t.fieldMask)};else{if(!(t instanceof kM))return $t();e={verify:Il(i,t.key)}}return t.fieldTransforms.length>0&&(e.updateTransforms=t.fieldTransforms.map(n=>function(s,o){const a=o.transform;if(a instanceof Oa)return{fieldPath:o.field.canonicalString(),setToServerValue:"REQUEST_TIME"};if(a instanceof uo)return{fieldPath:o.field.canonicalString(),appendMissingElements:{values:a.elements}};if(a instanceof fo)return{fieldPath:o.field.canonicalString(),removeAllFromArray:{values:a.elements}};if(a instanceof Fa)return{fieldPath:o.field.canonicalString(),increment:a.Pe};throw $t()}(0,n))),t.precondition.isNone||(e.currentDocument=function(r,s){return s.updateTime!==void 0?{updateTime:ZM(r,s.updateTime)}:s.exists!==void 0?{exists:s.exists}:$t()}(i,t.precondition)),e}function nw(i,t){return i&&i.length>0?(ge(t!==void 0),i.map(e=>function(r,s){let o=r.updateTime?Qn(r.updateTime):Qn(s);return o.isEqual(Zt.min())&&(o=Qn(s)),new FM(o,r.transformResults||[])}(e,t))):[]}function iw(i,t){return{documents:[Mm(i,t.path)]}}function rw(i,t){const e={structuredQuery:{}},n=t.path;let r;t.collectionGroup!==null?(r=n,e.structuredQuery.from=[{collectionId:t.collectionGroup,allDescendants:!0}]):(r=n.popLast(),e.structuredQuery.from=[{collectionId:n.lastSegment()}]),e.parent=Mm(i,r);const s=function(h){if(h.length!==0)return Am(Xn.create(h,"and"))}(t.filters);s&&(e.structuredQuery.where=s);const o=function(h){if(h.length!==0)return h.map(u=>function(f){return{field:$r(f.field),direction:aw(f.dir)}}(u))}(t.orderBy);o&&(e.structuredQuery.orderBy=o);const a=Rl(i,t.limit);return a!==null&&(e.structuredQuery.limit=a),t.startAt&&(e.structuredQuery.startAt=function(h){return{before:h.inclusive,values:h.position}}(t.startAt)),t.endAt&&(e.structuredQuery.endAt=function(h){return{before:!h.inclusive,values:h.position}}(t.endAt)),{_t:e,parent:r}}function sw(i){let t=JM(i.parent);const e=i.structuredQuery,n=e.from?e.from.length:0;let r=null;if(n>0){ge(n===1);const u=e.from[0];u.allDescendants?r=u.collectionId:t=t.child(u.collectionId)}let s=[];e.where&&(s=function(d){const f=bm(d);return f instanceof Xn&&tm(f)?f.getFilters():[f]}(e.where));let o=[];e.orderBy&&(o=function(d){return d.map(f=>function(y){return new ho(Kr(y.field),function(_){switch(_){case"ASCENDING":return"asc";case"DESCENDING":return"desc";default:return}}(y.direction))}(f))}(e.orderBy));let a=null;e.limit&&(a=function(d){let f;return f=typeof d=="object"?d.value:d,$a(f)?null:f}(e.limit));let c=null;e.startAt&&(c=function(d){const f=!!d.before,p=d.values||[];return new Na(p,f)}(e.startAt));let h=null;return e.endAt&&(h=function(d){const f=!d.before,p=d.values||[];return new Na(p,f)}(e.endAt)),MM(t,r,o,s,a,"F",c,h)}function ow(i,t){const e=function(r){switch(r){case"TargetPurposeListen":return null;case"TargetPurposeExistenceFilterMismatch":return"existence-filter-mismatch";case"TargetPurposeExistenceFilterMismatchBloom":return"existence-filter-mismatch-bloom";case"TargetPurposeLimboResolution":return"limbo-document";default:return $t()}}(t.purpose);return e==null?null:{"goog-listen-tags":e}}function bm(i){return i.unaryFilter!==void 0?function(e){switch(e.unaryFilter.op){case"IS_NAN":const n=Kr(e.unaryFilter.field);return Ve.create(n,"==",{doubleValue:NaN});case"IS_NULL":const r=Kr(e.unaryFilter.field);return Ve.create(r,"==",{nullValue:"NULL_VALUE"});case"IS_NOT_NAN":const s=Kr(e.unaryFilter.field);return Ve.create(s,"!=",{doubleValue:NaN});case"IS_NOT_NULL":const o=Kr(e.unaryFilter.field);return Ve.create(o,"!=",{nullValue:"NULL_VALUE"});default:return $t()}}(i):i.fieldFilter!==void 0?function(e){return Ve.create(Kr(e.fieldFilter.field),function(r){switch(r){case"EQUAL":return"==";case"NOT_EQUAL":return"!=";case"GREATER_THAN":return">";case"GREATER_THAN_OR_EQUAL":return">=";case"LESS_THAN":return"<";case"LESS_THAN_OR_EQUAL":return"<=";case"ARRAY_CONTAINS":return"array-contains";case"IN":return"in";case"NOT_IN":return"not-in";case"ARRAY_CONTAINS_ANY":return"array-contains-any";default:return $t()}}(e.fieldFilter.op),e.fieldFilter.value)}(i):i.compositeFilter!==void 0?function(e){return Xn.create(e.compositeFilter.filters.map(n=>bm(n)),function(r){switch(r){case"AND":return"and";case"OR":return"or";default:return $t()}}(e.compositeFilter.op))}(i):$t()}function aw(i){return YM[i]}function cw(i){return $M[i]}function lw(i){return KM[i]}function $r(i){return{fieldPath:i.canonicalString()}}function Kr(i){return Je.fromServerFormat(i.fieldPath)}function Am(i){return i instanceof Ve?function(e){if(e.op==="=="){if(af(e.value))return{unaryFilter:{field:$r(e.field),op:"IS_NAN"}};if(of(e.value))return{unaryFilter:{field:$r(e.field),op:"IS_NULL"}}}else if(e.op==="!="){if(af(e.value))return{unaryFilter:{field:$r(e.field),op:"IS_NOT_NAN"}};if(of(e.value))return{unaryFilter:{field:$r(e.field),op:"IS_NOT_NULL"}}}return{fieldFilter:{field:$r(e.field),op:cw(e.op),value:e.value}}}(i):i instanceof Xn?function(e){const n=e.getFilters().map(r=>Am(r));return n.length===1?n[0]:{compositeFilter:{op:lw(e.op),filters:n}}}(i):$t()}function hw(i){const t=[];return i.fields.forEach(e=>t.push(e.canonicalString())),{fieldPaths:t}}function Rm(i){return i.length>=4&&i.get(0)==="projects"&&i.get(2)==="databases"}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class Pi{constructor(t,e,n,r,s=Zt.min(),o=Zt.min(),a=en.EMPTY_BYTE_STRING,c=null){this.target=t,this.targetId=e,this.purpose=n,this.sequenceNumber=r,this.snapshotVersion=s,this.lastLimboFreeSnapshotVersion=o,this.resumeToken=a,this.expectedCount=c}withSequenceNumber(t){return new Pi(this.target,this.targetId,this.purpose,t,this.snapshotVersion,this.lastLimboFreeSnapshotVersion,this.resumeToken,this.expectedCount)}withResumeToken(t,e){return new Pi(this.target,this.targetId,this.purpose,this.sequenceNumber,e,this.lastLimboFreeSnapshotVersion,t,null)}withExpectedCount(t){return new Pi(this.target,this.targetId,this.purpose,this.sequenceNumber,this.snapshotVersion,this.lastLimboFreeSnapshotVersion,this.resumeToken,t)}withLastLimboFreeSnapshotVersion(t){return new Pi(this.target,this.targetId,this.purpose,this.sequenceNumber,this.snapshotVersion,t,this.resumeToken,this.expectedCount)}}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class uw{constructor(t){this.ct=t}}function dw(i){const t=sw({parent:i.parent,structuredQuery:i.structuredQuery});return i.limitType==="LAST"?Ua(t,t.limit,"L"):t}/**
 * @license
 * Copyright 2019 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class fw{constructor(){this.un=new pw}addToCollectionParentIndex(t,e){return this.un.add(e),ot.resolve()}getCollectionParents(t,e){return ot.resolve(this.un.getEntries(e))}addFieldIndex(t,e){return ot.resolve()}deleteFieldIndex(t,e){return ot.resolve()}deleteAllFieldIndexes(t){return ot.resolve()}createTargetIndexes(t,e){return ot.resolve()}getDocumentsMatchingTarget(t,e){return ot.resolve(null)}getIndexType(t,e){return ot.resolve(0)}getFieldIndexes(t,e){return ot.resolve([])}getNextCollectionGroupToUpdate(t){return ot.resolve(null)}getMinOffset(t,e){return ot.resolve(ki.min())}getMinOffsetFromCollectionGroup(t,e){return ot.resolve(ki.min())}updateCollectionGroup(t,e,n){return ot.resolve()}updateIndexEntries(t,e){return ot.resolve()}}class pw{constructor(){this.index={}}add(t){const e=t.lastSegment(),n=t.popLast(),r=this.index[e]||new tn(be.comparator),s=!r.has(n);return this.index[e]=r.add(n),s}has(t){const e=t.lastSegment(),n=t.popLast(),r=this.index[e];return r&&r.has(n)}getEntries(t){return(this.index[t]||new tn(be.comparator)).toArray()}}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class fs{constructor(t){this.Ln=t}next(){return this.Ln+=2,this.Ln}static Bn(){return new fs(0)}static kn(){return new fs(-1)}}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class mw{constructor(){this.changes=new Ts(t=>t.toString(),(t,e)=>t.isEqual(e)),this.changesApplied=!1}addEntry(t){this.assertNotApplied(),this.changes.set(t.key,t)}removeEntry(t,e){this.assertNotApplied(),this.changes.set(t,un.newInvalidDocument(t).setReadTime(e))}getEntry(t,e){this.assertNotApplied();const n=this.changes.get(e);return n!==void 0?ot.resolve(n):this.getFromCache(t,e)}getEntries(t,e){return this.getAllFromCache(t,e)}apply(t){return this.assertNotApplied(),this.changesApplied=!0,this.applyChanges(t)}assertNotApplied(){}}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *//**
 * @license
 * Copyright 2022 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class gw{constructor(t,e){this.overlayedDocument=t,this.mutatedFields=e}}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class _w{constructor(t,e,n,r){this.remoteDocumentCache=t,this.mutationQueue=e,this.documentOverlayCache=n,this.indexManager=r}getDocument(t,e){let n=null;return this.documentOverlayCache.getOverlay(t,e).next(r=>(n=r,this.remoteDocumentCache.getEntry(t,e))).next(r=>(n!==null&&Js(n.mutation,r,qn.empty(),He.now()),r))}getDocuments(t,e){return this.remoteDocumentCache.getEntries(t,e).next(n=>this.getLocalViewOfDocuments(t,n,ce()).next(()=>n))}getLocalViewOfDocuments(t,e,n=ce()){const r=sr();return this.populateOverlays(t,r,e).next(()=>this.computeViews(t,e,r,n).next(s=>{let o=qs();return s.forEach((a,c)=>{o=o.insert(a,c.overlayedDocument)}),o}))}getOverlayedDocuments(t,e){const n=sr();return this.populateOverlays(t,n,e).next(()=>this.computeViews(t,e,n,ce()))}populateOverlays(t,e,n){const r=[];return n.forEach(s=>{e.has(s)||r.push(s)}),this.documentOverlayCache.getOverlays(t,r).next(s=>{s.forEach((o,a)=>{e.set(o,a)})})}computeViews(t,e,n,r){let s=gi();const o=Zs(),a=function(){return Zs()}();return e.forEach((c,h)=>{const u=n.get(h.key);r.has(h.key)&&(u===void 0||u.mutation instanceof _r)?s=s.insert(h.key,h):u!==void 0?(o.set(h.key,u.mutation.getFieldMask()),Js(u.mutation,h,u.mutation.getFieldMask(),He.now())):o.set(h.key,qn.empty())}),this.recalculateAndSaveOverlays(t,s).next(c=>(c.forEach((h,u)=>o.set(h,u)),e.forEach((h,u)=>{var d;return a.set(h,new gw(u,(d=o.get(h))!==null&&d!==void 0?d:null))}),a))}recalculateAndSaveOverlays(t,e){const n=Zs();let r=new Pe((o,a)=>o-a),s=ce();return this.mutationQueue.getAllMutationBatchesAffectingDocumentKeys(t,e).next(o=>{for(const a of o)a.keys().forEach(c=>{const h=e.get(c);if(h===null)return;let u=n.get(c)||qn.empty();u=a.applyToLocalView(h,u),n.set(c,u);const d=(r.get(a.batchId)||ce()).add(c);r=r.insert(a.batchId,d)})}).next(()=>{const o=[],a=r.getReverseIterator();for(;a.hasNext();){const c=a.getNext(),h=c.key,u=c.value,d=hm();u.forEach(f=>{if(!s.has(f)){const p=gm(e.get(f),n.get(f));p!==null&&d.set(f,p),s=s.add(f)}}),o.push(this.documentOverlayCache.saveOverlays(t,h,d))}return ot.waitFor(o)}).next(()=>n)}recalculateAndSaveOverlaysForDocumentKeys(t,e){return this.remoteDocumentCache.getEntries(t,e).next(n=>this.recalculateAndSaveOverlays(t,n))}getDocumentsMatchingQuery(t,e,n,r){return function(o){return Bt.isDocumentKey(o.path)&&o.collectionGroup===null&&o.filters.length===0}(e)?this.getDocumentsMatchingDocumentQuery(t,e.path):sm(e)?this.getDocumentsMatchingCollectionGroupQuery(t,e,n,r):this.getDocumentsMatchingCollectionQuery(t,e,n,r)}getNextDocuments(t,e,n,r){return this.remoteDocumentCache.getAllFromCollectionGroup(t,e,n,r).next(s=>{const o=r-s.size>0?this.documentOverlayCache.getOverlaysForCollectionGroup(t,e,n.largestBatchId,r-s.size):ot.resolve(sr());let a=-1,c=s;return o.next(h=>ot.forEach(h,(u,d)=>(a<d.largestBatchId&&(a=d.largestBatchId),s.get(u)?ot.resolve():this.remoteDocumentCache.getEntry(t,u).next(f=>{c=c.insert(u,f)}))).next(()=>this.populateOverlays(t,h,s)).next(()=>this.computeViews(t,c,h,ce())).next(u=>({batchId:a,changes:lm(u)})))})}getDocumentsMatchingDocumentQuery(t,e){return this.getDocument(t,new Bt(e)).next(n=>{let r=qs();return n.isFoundDocument()&&(r=r.insert(n.key,n)),r})}getDocumentsMatchingCollectionGroupQuery(t,e,n,r){const s=e.collectionGroup;let o=qs();return this.indexManager.getCollectionParents(t,s).next(a=>ot.forEach(a,c=>{const h=function(d,f){return new Ss(f,null,d.explicitOrderBy.slice(),d.filters.slice(),d.limit,d.limitType,d.startAt,d.endAt)}(e,c.child(s));return this.getDocumentsMatchingCollectionQuery(t,h,n,r).next(u=>{u.forEach((d,f)=>{o=o.insert(d,f)})})}).next(()=>o))}getDocumentsMatchingCollectionQuery(t,e,n,r){let s;return this.documentOverlayCache.getOverlaysForCollection(t,e.path,n.largestBatchId).next(o=>(s=o,this.remoteDocumentCache.getDocumentsMatchingQuery(t,e,n,s,r))).next(o=>{s.forEach((c,h)=>{const u=h.getKey();o.get(u)===null&&(o=o.insert(u,un.newInvalidDocument(u)))});let a=qs();return o.forEach((c,h)=>{const u=s.get(c);u!==void 0&&Js(u.mutation,h,qn.empty(),He.now()),Qa(e,h)&&(a=a.insert(c,h))}),a})}}/**
 * @license
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class vw{constructor(t){this.serializer=t,this.hr=new Map,this.Pr=new Map}getBundleMetadata(t,e){return ot.resolve(this.hr.get(e))}saveBundleMetadata(t,e){return this.hr.set(e.id,function(r){return{id:r.id,version:r.version,createTime:Qn(r.createTime)}}(e)),ot.resolve()}getNamedQuery(t,e){return ot.resolve(this.Pr.get(e))}saveNamedQuery(t,e){return this.Pr.set(e.name,function(r){return{name:r.name,query:dw(r.bundledQuery),readTime:Qn(r.readTime)}}(e)),ot.resolve()}}/**
 * @license
 * Copyright 2022 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class yw{constructor(){this.overlays=new Pe(Bt.comparator),this.Ir=new Map}getOverlay(t,e){return ot.resolve(this.overlays.get(e))}getOverlays(t,e){const n=sr();return ot.forEach(e,r=>this.getOverlay(t,r).next(s=>{s!==null&&n.set(r,s)})).next(()=>n)}saveOverlays(t,e,n){return n.forEach((r,s)=>{this.ht(t,e,s)}),ot.resolve()}removeOverlaysForBatchId(t,e,n){const r=this.Ir.get(n);return r!==void 0&&(r.forEach(s=>this.overlays=this.overlays.remove(s)),this.Ir.delete(n)),ot.resolve()}getOverlaysForCollection(t,e,n){const r=sr(),s=e.length+1,o=new Bt(e.child("")),a=this.overlays.getIteratorFrom(o);for(;a.hasNext();){const c=a.getNext().value,h=c.getKey();if(!e.isPrefixOf(h.path))break;h.path.length===s&&c.largestBatchId>n&&r.set(c.getKey(),c)}return ot.resolve(r)}getOverlaysForCollectionGroup(t,e,n,r){let s=new Pe((h,u)=>h-u);const o=this.overlays.getIterator();for(;o.hasNext();){const h=o.getNext().value;if(h.getKey().getCollectionGroup()===e&&h.largestBatchId>n){let u=s.get(h.largestBatchId);u===null&&(u=sr(),s=s.insert(h.largestBatchId,u)),u.set(h.getKey(),h)}}const a=sr(),c=s.getIterator();for(;c.hasNext()&&(c.getNext().value.forEach((h,u)=>a.set(h,u)),!(a.size()>=r)););return ot.resolve(a)}ht(t,e,n){const r=this.overlays.get(n.key);if(r!==null){const o=this.Ir.get(r.largestBatchId).delete(n.key);this.Ir.set(r.largestBatchId,o)}this.overlays=this.overlays.insert(n.key,new zM(e,n));let s=this.Ir.get(e);s===void 0&&(s=ce(),this.Ir.set(e,s)),this.Ir.set(e,s.add(n.key))}}/**
 * @license
 * Copyright 2024 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class Ew{constructor(){this.sessionToken=en.EMPTY_BYTE_STRING}getSessionToken(t){return ot.resolve(this.sessionToken)}setSessionToken(t,e){return this.sessionToken=e,ot.resolve()}}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class ah{constructor(){this.Tr=new tn(qe.Er),this.dr=new tn(qe.Ar)}isEmpty(){return this.Tr.isEmpty()}addReference(t,e){const n=new qe(t,e);this.Tr=this.Tr.add(n),this.dr=this.dr.add(n)}Rr(t,e){t.forEach(n=>this.addReference(n,e))}removeReference(t,e){this.Vr(new qe(t,e))}mr(t,e){t.forEach(n=>this.removeReference(n,e))}gr(t){const e=new Bt(new be([])),n=new qe(e,t),r=new qe(e,t+1),s=[];return this.dr.forEachInRange([n,r],o=>{this.Vr(o),s.push(o.key)}),s}pr(){this.Tr.forEach(t=>this.Vr(t))}Vr(t){this.Tr=this.Tr.delete(t),this.dr=this.dr.delete(t)}yr(t){const e=new Bt(new be([])),n=new qe(e,t),r=new qe(e,t+1);let s=ce();return this.dr.forEachInRange([n,r],o=>{s=s.add(o.key)}),s}containsKey(t){const e=new qe(t,0),n=this.Tr.firstAfterOrEqual(e);return n!==null&&t.isEqual(n.key)}}class qe{constructor(t,e){this.key=t,this.wr=e}static Er(t,e){return Bt.comparator(t.key,e.key)||fe(t.wr,e.wr)}static Ar(t,e){return fe(t.wr,e.wr)||Bt.comparator(t.key,e.key)}}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class xw{constructor(t,e){this.indexManager=t,this.referenceDelegate=e,this.mutationQueue=[],this.Sr=1,this.br=new tn(qe.Er)}checkEmpty(t){return ot.resolve(this.mutationQueue.length===0)}addMutationBatch(t,e,n,r){const s=this.Sr;this.Sr++,this.mutationQueue.length>0&&this.mutationQueue[this.mutationQueue.length-1];const o=new HM(s,e,n,r);this.mutationQueue.push(o);for(const a of r)this.br=this.br.add(new qe(a.key,s)),this.indexManager.addToCollectionParentIndex(t,a.key.path.popLast());return ot.resolve(o)}lookupMutationBatch(t,e){return ot.resolve(this.Dr(e))}getNextMutationBatchAfterBatchId(t,e){const n=e+1,r=this.vr(n),s=r<0?0:r;return ot.resolve(this.mutationQueue.length>s?this.mutationQueue[s]:null)}getHighestUnacknowledgedBatchId(){return ot.resolve(this.mutationQueue.length===0?-1:this.Sr-1)}getAllMutationBatches(t){return ot.resolve(this.mutationQueue.slice())}getAllMutationBatchesAffectingDocumentKey(t,e){const n=new qe(e,0),r=new qe(e,Number.POSITIVE_INFINITY),s=[];return this.br.forEachInRange([n,r],o=>{const a=this.Dr(o.wr);s.push(a)}),ot.resolve(s)}getAllMutationBatchesAffectingDocumentKeys(t,e){let n=new tn(fe);return e.forEach(r=>{const s=new qe(r,0),o=new qe(r,Number.POSITIVE_INFINITY);this.br.forEachInRange([s,o],a=>{n=n.add(a.wr)})}),ot.resolve(this.Cr(n))}getAllMutationBatchesAffectingQuery(t,e){const n=e.path,r=n.length+1;let s=n;Bt.isDocumentKey(s)||(s=s.child(""));const o=new qe(new Bt(s),0);let a=new tn(fe);return this.br.forEachWhile(c=>{const h=c.key.path;return!!n.isPrefixOf(h)&&(h.length===r&&(a=a.add(c.wr)),!0)},o),ot.resolve(this.Cr(a))}Cr(t){const e=[];return t.forEach(n=>{const r=this.Dr(n);r!==null&&e.push(r)}),e}removeMutationBatch(t,e){ge(this.Fr(e.batchId,"removed")===0),this.mutationQueue.shift();let n=this.br;return ot.forEach(e.mutations,r=>{const s=new qe(r.key,e.batchId);return n=n.delete(s),this.referenceDelegate.markPotentiallyOrphaned(t,r.key)}).next(()=>{this.br=n})}On(t){}containsKey(t,e){const n=new qe(e,0),r=this.br.firstAfterOrEqual(n);return ot.resolve(e.isEqual(r&&r.key))}performConsistencyCheck(t){return this.mutationQueue.length,ot.resolve()}Fr(t,e){return this.vr(t)}vr(t){return this.mutationQueue.length===0?0:t-this.mutationQueue[0].batchId}Dr(t){const e=this.vr(t);return e<0||e>=this.mutationQueue.length?null:this.mutationQueue[e]}}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class Sw{constructor(t){this.Mr=t,this.docs=function(){return new Pe(Bt.comparator)}(),this.size=0}setIndexManager(t){this.indexManager=t}addEntry(t,e){const n=e.key,r=this.docs.get(n),s=r?r.size:0,o=this.Mr(e);return this.docs=this.docs.insert(n,{document:e.mutableCopy(),size:o}),this.size+=o-s,this.indexManager.addToCollectionParentIndex(t,n.path.popLast())}removeEntry(t){const e=this.docs.get(t);e&&(this.docs=this.docs.remove(t),this.size-=e.size)}getEntry(t,e){const n=this.docs.get(e);return ot.resolve(n?n.document.mutableCopy():un.newInvalidDocument(e))}getEntries(t,e){let n=gi();return e.forEach(r=>{const s=this.docs.get(r);n=n.insert(r,s?s.document.mutableCopy():un.newInvalidDocument(r))}),ot.resolve(n)}getDocumentsMatchingQuery(t,e,n,r){let s=gi();const o=e.path,a=new Bt(o.child("")),c=this.docs.getIteratorFrom(a);for(;c.hasNext();){const{key:h,value:{document:u}}=c.getNext();if(!o.isPrefixOf(h.path))break;h.path.length>o.length+1||sM(rM(u),n)<=0||(r.has(u.key)||Qa(e,u))&&(s=s.insert(u.key,u.mutableCopy()))}return ot.resolve(s)}getAllFromCollectionGroup(t,e,n,r){$t()}Or(t,e){return ot.forEach(this.docs,n=>e(n))}newChangeBuffer(t){return new Tw(this)}getSize(t){return ot.resolve(this.size)}}class Tw extends mw{constructor(t){super(),this.cr=t}applyChanges(t){const e=[];return this.changes.forEach((n,r)=>{r.isValidDocument()?e.push(this.cr.addEntry(t,r)):this.cr.removeEntry(n)}),ot.waitFor(e)}getFromCache(t,e){return this.cr.getEntry(t,e)}getAllFromCache(t,e){return this.cr.getEntries(t,e)}}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class Mw{constructor(t){this.persistence=t,this.Nr=new Ts(e=>eh(e),nh),this.lastRemoteSnapshotVersion=Zt.min(),this.highestTargetId=0,this.Lr=0,this.Br=new ah,this.targetCount=0,this.kr=fs.Bn()}forEachTarget(t,e){return this.Nr.forEach((n,r)=>e(r)),ot.resolve()}getLastRemoteSnapshotVersion(t){return ot.resolve(this.lastRemoteSnapshotVersion)}getHighestSequenceNumber(t){return ot.resolve(this.Lr)}allocateTargetId(t){return this.highestTargetId=this.kr.next(),ot.resolve(this.highestTargetId)}setTargetsMetadata(t,e,n){return n&&(this.lastRemoteSnapshotVersion=n),e>this.Lr&&(this.Lr=e),ot.resolve()}Kn(t){this.Nr.set(t.target,t);const e=t.targetId;e>this.highestTargetId&&(this.kr=new fs(e),this.highestTargetId=e),t.sequenceNumber>this.Lr&&(this.Lr=t.sequenceNumber)}addTargetData(t,e){return this.Kn(e),this.targetCount+=1,ot.resolve()}updateTargetData(t,e){return this.Kn(e),ot.resolve()}removeTargetData(t,e){return this.Nr.delete(e.target),this.Br.gr(e.targetId),this.targetCount-=1,ot.resolve()}removeTargets(t,e,n){let r=0;const s=[];return this.Nr.forEach((o,a)=>{a.sequenceNumber<=e&&n.get(a.targetId)===null&&(this.Nr.delete(o),s.push(this.removeMatchingKeysForTargetId(t,a.targetId)),r++)}),ot.waitFor(s).next(()=>r)}getTargetCount(t){return ot.resolve(this.targetCount)}getTargetData(t,e){const n=this.Nr.get(e)||null;return ot.resolve(n)}addMatchingKeys(t,e,n){return this.Br.Rr(e,n),ot.resolve()}removeMatchingKeys(t,e,n){this.Br.mr(e,n);const r=this.persistence.referenceDelegate,s=[];return r&&e.forEach(o=>{s.push(r.markPotentiallyOrphaned(t,o))}),ot.waitFor(s)}removeMatchingKeysForTargetId(t,e){return this.Br.gr(e),ot.resolve()}getMatchingKeysForTargetId(t,e){const n=this.Br.yr(e);return ot.resolve(n)}containsKey(t,e){return ot.resolve(this.Br.containsKey(e))}}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class ww{constructor(t,e){this.qr={},this.overlays={},this.Qr=new Ql(0),this.Kr=!1,this.Kr=!0,this.$r=new Ew,this.referenceDelegate=t(this),this.Ur=new Mw(this),this.indexManager=new fw,this.remoteDocumentCache=function(r){return new Sw(r)}(n=>this.referenceDelegate.Wr(n)),this.serializer=new uw(e),this.Gr=new vw(this.serializer)}start(){return Promise.resolve()}shutdown(){return this.Kr=!1,Promise.resolve()}get started(){return this.Kr}setDatabaseDeletedListener(){}setNetworkEnabled(){}getIndexManager(t){return this.indexManager}getDocumentOverlayCache(t){let e=this.overlays[t.toKey()];return e||(e=new yw,this.overlays[t.toKey()]=e),e}getMutationQueue(t,e){let n=this.qr[t.toKey()];return n||(n=new xw(e,this.referenceDelegate),this.qr[t.toKey()]=n),n}getGlobalsCache(){return this.$r}getTargetCache(){return this.Ur}getRemoteDocumentCache(){return this.remoteDocumentCache}getBundleCache(){return this.Gr}runTransaction(t,e,n){At("MemoryPersistence","Starting transaction:",t);const r=new bw(this.Qr.next());return this.referenceDelegate.zr(),n(r).next(s=>this.referenceDelegate.jr(r).next(()=>s)).toPromise().then(s=>(r.raiseOnCommittedEvent(),s))}Hr(t,e){return ot.or(Object.values(this.qr).map(n=>()=>n.containsKey(t,e)))}}class bw extends aM{constructor(t){super(),this.currentSequenceNumber=t}}class ch{constructor(t){this.persistence=t,this.Jr=new ah,this.Yr=null}static Zr(t){return new ch(t)}get Xr(){if(this.Yr)return this.Yr;throw $t()}addReference(t,e,n){return this.Jr.addReference(n,e),this.Xr.delete(n.toString()),ot.resolve()}removeReference(t,e,n){return this.Jr.removeReference(n,e),this.Xr.add(n.toString()),ot.resolve()}markPotentiallyOrphaned(t,e){return this.Xr.add(e.toString()),ot.resolve()}removeTarget(t,e){this.Jr.gr(e.targetId).forEach(r=>this.Xr.add(r.toString()));const n=this.persistence.getTargetCache();return n.getMatchingKeysForTargetId(t,e.targetId).next(r=>{r.forEach(s=>this.Xr.add(s.toString()))}).next(()=>n.removeTargetData(t,e))}zr(){this.Yr=new Set}jr(t){const e=this.persistence.getRemoteDocumentCache().newChangeBuffer();return ot.forEach(this.Xr,n=>{const r=Bt.fromPath(n);return this.ei(t,r).next(s=>{s||e.removeEntry(r,Zt.min())})}).next(()=>(this.Yr=null,e.apply(t)))}updateLimboDocument(t,e){return this.ei(t,e).next(n=>{n?this.Xr.delete(e.toString()):this.Xr.add(e.toString())})}Wr(t){return 0}ei(t,e){return ot.or([()=>ot.resolve(this.Jr.containsKey(e)),()=>this.persistence.getTargetCache().containsKey(t,e),()=>this.persistence.Hr(t,e)])}}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class lh{constructor(t,e,n,r){this.targetId=t,this.fromCache=e,this.$i=n,this.Ui=r}static Wi(t,e){let n=ce(),r=ce();for(const s of e.docChanges)switch(s.type){case 0:n=n.add(s.doc.key);break;case 1:r=r.add(s.doc.key)}return new lh(t,e.fromCache,n,r)}}/**
 * @license
 * Copyright 2023 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class Aw{constructor(){this._documentReadCount=0}get documentReadCount(){return this._documentReadCount}incrementDocumentReadCount(t){this._documentReadCount+=t}}/**
 * @license
 * Copyright 2019 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class Rw{constructor(){this.Gi=!1,this.zi=!1,this.ji=100,this.Hi=function(){return IS()?8:cM(RS())>0?6:4}()}initialize(t,e){this.Ji=t,this.indexManager=e,this.Gi=!0}getDocumentsMatchingQuery(t,e,n,r){const s={result:null};return this.Yi(t,e).next(o=>{s.result=o}).next(()=>{if(!s.result)return this.Zi(t,e,r,n).next(o=>{s.result=o})}).next(()=>{if(s.result)return;const o=new Aw;return this.Xi(t,e,o).next(a=>{if(s.result=a,this.zi)return this.es(t,e,o,a.size)})}).next(()=>s.result)}es(t,e,n,r){return n.documentReadCount<this.ji?(Gs()<=ue.DEBUG&&At("QueryEngine","SDK will not create cache indexes for query:",Yr(e),"since it only creates cache indexes for collection contains","more than or equal to",this.ji,"documents"),ot.resolve()):(Gs()<=ue.DEBUG&&At("QueryEngine","Query:",Yr(e),"scans",n.documentReadCount,"local documents and returns",r,"documents as results."),n.documentReadCount>this.Hi*r?(Gs()<=ue.DEBUG&&At("QueryEngine","The SDK decides to create cache indexes for query:",Yr(e),"as using cache indexes may help improve performance."),this.indexManager.createTargetIndexes(t,Kn(e))):ot.resolve())}Yi(t,e){if(uf(e))return ot.resolve(null);let n=Kn(e);return this.indexManager.getIndexType(t,n).next(r=>r===0?null:(e.limit!==null&&r===1&&(e=Ua(e,null,"F"),n=Kn(e)),this.indexManager.getDocumentsMatchingTarget(t,n).next(s=>{const o=ce(...s);return this.Ji.getDocuments(t,o).next(a=>this.indexManager.getMinOffset(t,n).next(c=>{const h=this.ts(e,a);return this.ns(e,h,o,c.readTime)?this.Yi(t,Ua(e,null,"F")):this.rs(t,h,e,c)}))})))}Zi(t,e,n,r){return uf(e)||r.isEqual(Zt.min())?ot.resolve(null):this.Ji.getDocuments(t,n).next(s=>{const o=this.ts(e,s);return this.ns(e,o,n,r)?ot.resolve(null):(Gs()<=ue.DEBUG&&At("QueryEngine","Re-using previous result from %s to execute query: %s",r.toString(),Yr(e)),this.rs(t,o,e,iM(r,-1)).next(a=>a))})}ts(t,e){let n=new tn(am(t));return e.forEach((r,s)=>{Qa(t,s)&&(n=n.add(s))}),n}ns(t,e,n,r){if(t.limit===null)return!1;if(n.size!==e.size)return!0;const s=t.limitType==="F"?e.last():e.first();return!!s&&(s.hasPendingWrites||s.version.compareTo(r)>0)}Xi(t,e,n){return Gs()<=ue.DEBUG&&At("QueryEngine","Using full collection scan to execute query:",Yr(e)),this.Ji.getDocumentsMatchingQuery(t,e,ki.min(),n)}rs(t,e,n,r){return this.Ji.getDocumentsMatchingQuery(t,n,r).next(s=>(e.forEach(o=>{s=s.insert(o.key,o)}),s))}}/**
 * @license
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class Cw{constructor(t,e,n,r){this.persistence=t,this.ss=e,this.serializer=r,this.os=new Pe(fe),this._s=new Ts(s=>eh(s),nh),this.us=new Map,this.cs=t.getRemoteDocumentCache(),this.Ur=t.getTargetCache(),this.Gr=t.getBundleCache(),this.ls(n)}ls(t){this.documentOverlayCache=this.persistence.getDocumentOverlayCache(t),this.indexManager=this.persistence.getIndexManager(t),this.mutationQueue=this.persistence.getMutationQueue(t,this.indexManager),this.localDocuments=new _w(this.cs,this.mutationQueue,this.documentOverlayCache,this.indexManager),this.cs.setIndexManager(this.indexManager),this.ss.initialize(this.localDocuments,this.indexManager)}collectGarbage(t){return this.persistence.runTransaction("Collect garbage","readwrite-primary",e=>t.collect(e,this.os))}}function Iw(i,t,e,n){return new Cw(i,t,e,n)}async function Cm(i,t){const e=Jt(i);return await e.persistence.runTransaction("Handle user change","readonly",n=>{let r;return e.mutationQueue.getAllMutationBatches(n).next(s=>(r=s,e.ls(t),e.mutationQueue.getAllMutationBatches(n))).next(s=>{const o=[],a=[];let c=ce();for(const h of r){o.push(h.batchId);for(const u of h.mutations)c=c.add(u.key)}for(const h of s){a.push(h.batchId);for(const u of h.mutations)c=c.add(u.key)}return e.localDocuments.getDocuments(n,c).next(h=>({hs:h,removedBatchIds:o,addedBatchIds:a}))})})}function Pw(i,t){const e=Jt(i);return e.persistence.runTransaction("Acknowledge batch","readwrite-primary",n=>{const r=t.batch.keys(),s=e.cs.newChangeBuffer({trackRemovals:!0});return function(a,c,h,u){const d=h.batch,f=d.keys();let p=ot.resolve();return f.forEach(y=>{p=p.next(()=>u.getEntry(c,y)).next(E=>{const _=h.docVersions.get(y);ge(_!==null),E.version.compareTo(_)<0&&(d.applyToRemoteDocument(E,h),E.isValidDocument()&&(E.setReadTime(h.commitVersion),u.addEntry(E)))})}),p.next(()=>a.mutationQueue.removeMutationBatch(c,d))}(e,n,t,s).next(()=>s.apply(n)).next(()=>e.mutationQueue.performConsistencyCheck(n)).next(()=>e.documentOverlayCache.removeOverlaysForBatchId(n,r,t.batch.batchId)).next(()=>e.localDocuments.recalculateAndSaveOverlaysForDocumentKeys(n,function(a){let c=ce();for(let h=0;h<a.mutationResults.length;++h)a.mutationResults[h].transformResults.length>0&&(c=c.add(a.batch.mutations[h].key));return c}(t))).next(()=>e.localDocuments.getDocuments(n,r))})}function Im(i){const t=Jt(i);return t.persistence.runTransaction("Get last remote snapshot version","readonly",e=>t.Ur.getLastRemoteSnapshotVersion(e))}function Dw(i,t){const e=Jt(i),n=t.snapshotVersion;let r=e.os;return e.persistence.runTransaction("Apply remote event","readwrite-primary",s=>{const o=e.cs.newChangeBuffer({trackRemovals:!0});r=e.os;const a=[];t.targetChanges.forEach((u,d)=>{const f=r.get(d);if(!f)return;a.push(e.Ur.removeMatchingKeys(s,u.removedDocuments,d).next(()=>e.Ur.addMatchingKeys(s,u.addedDocuments,d)));let p=f.withSequenceNumber(s.currentSequenceNumber);t.targetMismatches.get(d)!==null?p=p.withResumeToken(en.EMPTY_BYTE_STRING,Zt.min()).withLastLimboFreeSnapshotVersion(Zt.min()):u.resumeToken.approximateByteSize()>0&&(p=p.withResumeToken(u.resumeToken,n)),r=r.insert(d,p),function(E,_,m){return E.resumeToken.approximateByteSize()===0||_.snapshotVersion.toMicroseconds()-E.snapshotVersion.toMicroseconds()>=3e8?!0:m.addedDocuments.size+m.modifiedDocuments.size+m.removedDocuments.size>0}(f,p,u)&&a.push(e.Ur.updateTargetData(s,p))});let c=gi(),h=ce();if(t.documentUpdates.forEach(u=>{t.resolvedLimboDocuments.has(u)&&a.push(e.persistence.referenceDelegate.updateLimboDocument(s,u))}),a.push(Lw(s,o,t.documentUpdates).next(u=>{c=u.Ps,h=u.Is})),!n.isEqual(Zt.min())){const u=e.Ur.getLastRemoteSnapshotVersion(s).next(d=>e.Ur.setTargetsMetadata(s,s.currentSequenceNumber,n));a.push(u)}return ot.waitFor(a).next(()=>o.apply(s)).next(()=>e.localDocuments.getLocalViewOfDocuments(s,c,h)).next(()=>c)}).then(s=>(e.os=r,s))}function Lw(i,t,e){let n=ce(),r=ce();return e.forEach(s=>n=n.add(s)),t.getEntries(i,n).next(s=>{let o=gi();return e.forEach((a,c)=>{const h=s.get(a);c.isFoundDocument()!==h.isFoundDocument()&&(r=r.add(a)),c.isNoDocument()&&c.version.isEqual(Zt.min())?(t.removeEntry(a,c.readTime),o=o.insert(a,c)):!h.isValidDocument()||c.version.compareTo(h.version)>0||c.version.compareTo(h.version)===0&&h.hasPendingWrites?(t.addEntry(c),o=o.insert(a,c)):At("LocalStore","Ignoring outdated watch update for ",a,". Current version:",h.version," Watch version:",c.version)}),{Ps:o,Is:r}})}function Nw(i,t){const e=Jt(i);return e.persistence.runTransaction("Get next mutation batch","readonly",n=>(t===void 0&&(t=-1),e.mutationQueue.getNextMutationBatchAfterBatchId(n,t)))}function Uw(i,t){const e=Jt(i);return e.persistence.runTransaction("Allocate target","readwrite",n=>{let r;return e.Ur.getTargetData(n,t).next(s=>s?(r=s,ot.resolve(r)):e.Ur.allocateTargetId(n).next(o=>(r=new Pi(t,o,"TargetPurposeListen",n.currentSequenceNumber),e.Ur.addTargetData(n,r).next(()=>r))))}).then(n=>{const r=e.os.get(n.targetId);return(r===null||n.snapshotVersion.compareTo(r.snapshotVersion)>0)&&(e.os=e.os.insert(n.targetId,n),e._s.set(t,n.targetId)),n})}async function Dl(i,t,e){const n=Jt(i),r=n.os.get(t),s=e?"readwrite":"readwrite-primary";try{e||await n.persistence.runTransaction("Release target",s,o=>n.persistence.referenceDelegate.removeTarget(o,r))}catch(o){if(!_o(o))throw o;At("LocalStore",`Failed to update sequence numbers for target ${t}: ${o}`)}n.os=n.os.remove(t),n._s.delete(r.target)}function Sf(i,t,e){const n=Jt(i);let r=Zt.min(),s=ce();return n.persistence.runTransaction("Execute query","readwrite",o=>function(c,h,u){const d=Jt(c),f=d._s.get(u);return f!==void 0?ot.resolve(d.os.get(f)):d.Ur.getTargetData(h,u)}(n,o,Kn(t)).next(a=>{if(a)return r=a.lastLimboFreeSnapshotVersion,n.Ur.getMatchingKeysForTargetId(o,a.targetId).next(c=>{s=c})}).next(()=>n.ss.getDocumentsMatchingQuery(o,t,e?r:Zt.min(),e?s:ce())).next(a=>(Ow(n,bM(t),a),{documents:a,Ts:s})))}function Ow(i,t,e){let n=i.us.get(t)||Zt.min();e.forEach((r,s)=>{s.readTime.compareTo(n)>0&&(n=s.readTime)}),i.us.set(t,n)}class Tf{constructor(){this.activeTargetIds=DM()}fs(t){this.activeTargetIds=this.activeTargetIds.add(t)}gs(t){this.activeTargetIds=this.activeTargetIds.delete(t)}Vs(){const t={activeTargetIds:this.activeTargetIds.toArray(),updateTimeMs:Date.now()};return JSON.stringify(t)}}class Fw{constructor(){this.so=new Tf,this.oo={},this.onlineStateHandler=null,this.sequenceNumberHandler=null}addPendingMutation(t){}updateMutationState(t,e,n){}addLocalQueryTarget(t,e=!0){return e&&this.so.fs(t),this.oo[t]||"not-current"}updateQueryState(t,e,n){this.oo[t]=e}removeLocalQueryTarget(t){this.so.gs(t)}isLocalQueryTarget(t){return this.so.activeTargetIds.has(t)}clearQueryState(t){delete this.oo[t]}getAllActiveQueryTargets(){return this.so.activeTargetIds}isActiveQueryTarget(t){return this.so.activeTargetIds.has(t)}start(){return this.so=new Tf,Promise.resolve()}handleUserChange(t,e,n){}setOnlineState(t){}shutdown(){}writeSequenceNumber(t){}notifyBundleLoaded(t){}}/**
 * @license
 * Copyright 2019 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class Vw{_o(t){}shutdown(){}}/**
 * @license
 * Copyright 2019 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class Mf{constructor(){this.ao=()=>this.uo(),this.co=()=>this.lo(),this.ho=[],this.Po()}_o(t){this.ho.push(t)}shutdown(){window.removeEventListener("online",this.ao),window.removeEventListener("offline",this.co)}Po(){window.addEventListener("online",this.ao),window.addEventListener("offline",this.co)}uo(){At("ConnectivityMonitor","Network connectivity changed: AVAILABLE");for(const t of this.ho)t(0)}lo(){At("ConnectivityMonitor","Network connectivity changed: UNAVAILABLE");for(const t of this.ho)t(1)}static D(){return typeof window<"u"&&window.addEventListener!==void 0&&window.removeEventListener!==void 0}}/**
 * @license
 * Copyright 2023 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */let ua=null;function tl(){return ua===null?ua=function(){return 268435456+Math.round(2147483648*Math.random())}():ua++,"0x"+ua.toString(16)}/**
 * @license
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */const Bw={BatchGetDocuments:"batchGet",Commit:"commit",RunQuery:"runQuery",RunAggregationQuery:"runAggregationQuery"};/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class kw{constructor(t){this.Io=t.Io,this.To=t.To}Eo(t){this.Ao=t}Ro(t){this.Vo=t}mo(t){this.fo=t}onMessage(t){this.po=t}close(){this.To()}send(t){this.Io(t)}yo(){this.Ao()}wo(){this.Vo()}So(t){this.fo(t)}bo(t){this.po(t)}}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */const cn="WebChannelConnection";class Hw extends class{constructor(e){this.databaseInfo=e,this.databaseId=e.databaseId;const n=e.ssl?"https":"http",r=encodeURIComponent(this.databaseId.projectId),s=encodeURIComponent(this.databaseId.database);this.Do=n+"://"+e.host,this.vo=`projects/${r}/databases/${s}`,this.Co=this.databaseId.database==="(default)"?`project_id=${r}`:`project_id=${r}&database_id=${s}`}get Fo(){return!1}Mo(e,n,r,s,o){const a=tl(),c=this.xo(e,n.toUriEncodedString());At("RestConnection",`Sending RPC '${e}' ${a}:`,c,r);const h={"google-cloud-resource-prefix":this.vo,"x-goog-request-params":this.Co};return this.Oo(h,s,o),this.No(e,c,h,r).then(u=>(At("RestConnection",`Received RPC '${e}' ${a}: `,u),u),u=>{throw ls("RestConnection",`RPC '${e}' ${a} failed with error: `,u,"url: ",c,"request:",r),u})}Lo(e,n,r,s,o,a){return this.Mo(e,n,r,s,o)}Oo(e,n,r){e["X-Goog-Api-Client"]=function(){return"gl-js/ fire/"+Es}(),e["Content-Type"]="text/plain",this.databaseInfo.appId&&(e["X-Firebase-GMPID"]=this.databaseInfo.appId),n&&n.headers.forEach((s,o)=>e[o]=s),r&&r.headers.forEach((s,o)=>e[o]=s)}xo(e,n){const r=Bw[e];return`${this.Do}/v1/${n}:${r}`}terminate(){}}{constructor(t){super(t),this.forceLongPolling=t.forceLongPolling,this.autoDetectLongPolling=t.autoDetectLongPolling,this.useFetchStreams=t.useFetchStreams,this.longPollingOptions=t.longPollingOptions}No(t,e,n,r){const s=tl();return new Promise((o,a)=>{const c=new zp;c.setWithCredentials(!0),c.listenOnce(Gp.COMPLETE,()=>{try{switch(c.getLastErrorCode()){case ma.NO_ERROR:const u=c.getResponseJson();At(cn,`XHR for RPC '${t}' ${s} received:`,JSON.stringify(u)),o(u);break;case ma.TIMEOUT:At(cn,`RPC '${t}' ${s} timed out`),a(new Tt(nt.DEADLINE_EXCEEDED,"Request time out"));break;case ma.HTTP_ERROR:const d=c.getStatus();if(At(cn,`RPC '${t}' ${s} failed with status:`,d,"response text:",c.getResponseText()),d>0){let f=c.getResponseJson();Array.isArray(f)&&(f=f[0]);const p=f==null?void 0:f.error;if(p&&p.status&&p.message){const y=function(_){const m=_.toLowerCase().replace(/_/g,"-");return Object.values(nt).indexOf(m)>=0?m:nt.UNKNOWN}(p.status);a(new Tt(y,p.message))}else a(new Tt(nt.UNKNOWN,"Server responded with status "+c.getStatus()))}else a(new Tt(nt.UNAVAILABLE,"Connection failed."));break;default:$t()}}finally{At(cn,`RPC '${t}' ${s} completed.`)}});const h=JSON.stringify(r);At(cn,`RPC '${t}' ${s} sending request:`,r),c.send(e,"POST",h,n,15)})}Bo(t,e,n){const r=tl(),s=[this.Do,"/","google.firestore.v1.Firestore","/",t,"/channel"],o=Xp(),a=qp(),c={httpSessionIdParam:"gsessionid",initMessageHeaders:{},messageUrlParams:{database:`projects/${this.databaseId.projectId}/databases/${this.databaseId.database}`},sendRawJson:!0,supportsCrossDomainXhr:!0,internalChannelParams:{forwardChannelRequestTimeoutMs:6e5},forceLongPolling:this.forceLongPolling,detectBufferingProxy:this.autoDetectLongPolling},h=this.longPollingOptions.timeoutSeconds;h!==void 0&&(c.longPollingTimeout=Math.round(1e3*h)),this.useFetchStreams&&(c.useFetchStreams=!0),this.Oo(c.initMessageHeaders,e,n),c.encodeInitMessageHeaders=!0;const u=s.join("");At(cn,`Creating RPC '${t}' stream ${r}: ${u}`,c);const d=o.createWebChannel(u,c);let f=!1,p=!1;const y=new kw({Io:_=>{p?At(cn,`Not sending because RPC '${t}' stream ${r} is closed:`,_):(f||(At(cn,`Opening RPC '${t}' stream ${r} transport.`),d.open(),f=!0),At(cn,`RPC '${t}' stream ${r} sending:`,_),d.send(_))},To:()=>d.close()}),E=(_,m,C)=>{_.listen(m,w=>{try{C(w)}catch(b){setTimeout(()=>{throw b},0)}})};return E(d,Ws.EventType.OPEN,()=>{p||(At(cn,`RPC '${t}' stream ${r} transport opened.`),y.yo())}),E(d,Ws.EventType.CLOSE,()=>{p||(p=!0,At(cn,`RPC '${t}' stream ${r} transport closed`),y.So())}),E(d,Ws.EventType.ERROR,_=>{p||(p=!0,ls(cn,`RPC '${t}' stream ${r} transport errored:`,_),y.So(new Tt(nt.UNAVAILABLE,"The operation could not be completed")))}),E(d,Ws.EventType.MESSAGE,_=>{var m;if(!p){const C=_.data[0];ge(!!C);const w=C,b=w.error||((m=w[0])===null||m===void 0?void 0:m.error);if(b){At(cn,`RPC '${t}' stream ${r} received error:`,b);const N=b.status;let U=function(v){const S=Fe[v];if(S!==void 0)return ym(S)}(N),T=b.message;U===void 0&&(U=nt.INTERNAL,T="Unknown error status: "+N+" with message "+b.message),p=!0,y.So(new Tt(U,T)),d.close()}else At(cn,`RPC '${t}' stream ${r} received:`,C),y.bo(C)}}),E(a,Wp.STAT_EVENT,_=>{_.stat===Sl.PROXY?At(cn,`RPC '${t}' stream ${r} detected buffering proxy`):_.stat===Sl.NOPROXY&&At(cn,`RPC '${t}' stream ${r} detected no buffering proxy`)}),setTimeout(()=>{y.wo()},0),y}}function el(){return typeof document<"u"?document:null}/**
 * @license
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */function ec(i){return new QM(i,!0)}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class Pm{constructor(t,e,n=1e3,r=1.5,s=6e4){this.ui=t,this.timerId=e,this.ko=n,this.qo=r,this.Qo=s,this.Ko=0,this.$o=null,this.Uo=Date.now(),this.reset()}reset(){this.Ko=0}Wo(){this.Ko=this.Qo}Go(t){this.cancel();const e=Math.floor(this.Ko+this.zo()),n=Math.max(0,Date.now()-this.Uo),r=Math.max(0,e-n);r>0&&At("ExponentialBackoff",`Backing off for ${r} ms (base delay: ${this.Ko} ms, delay with jitter: ${e} ms, last attempt: ${n} ms ago)`),this.$o=this.ui.enqueueAfterDelay(this.timerId,r,()=>(this.Uo=Date.now(),t())),this.Ko*=this.qo,this.Ko<this.ko&&(this.Ko=this.ko),this.Ko>this.Qo&&(this.Ko=this.Qo)}jo(){this.$o!==null&&(this.$o.skipDelay(),this.$o=null)}cancel(){this.$o!==null&&(this.$o.cancel(),this.$o=null)}zo(){return(Math.random()-.5)*this.Ko}}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class Dm{constructor(t,e,n,r,s,o,a,c){this.ui=t,this.Ho=n,this.Jo=r,this.connection=s,this.authCredentialsProvider=o,this.appCheckCredentialsProvider=a,this.listener=c,this.state=0,this.Yo=0,this.Zo=null,this.Xo=null,this.stream=null,this.e_=0,this.t_=new Pm(t,e)}n_(){return this.state===1||this.state===5||this.r_()}r_(){return this.state===2||this.state===3}start(){this.e_=0,this.state!==4?this.auth():this.i_()}async stop(){this.n_()&&await this.close(0)}s_(){this.state=0,this.t_.reset()}o_(){this.r_()&&this.Zo===null&&(this.Zo=this.ui.enqueueAfterDelay(this.Ho,6e4,()=>this.__()))}a_(t){this.u_(),this.stream.send(t)}async __(){if(this.r_())return this.close(0)}u_(){this.Zo&&(this.Zo.cancel(),this.Zo=null)}c_(){this.Xo&&(this.Xo.cancel(),this.Xo=null)}async close(t,e){this.u_(),this.c_(),this.t_.cancel(),this.Yo++,t!==4?this.t_.reset():e&&e.code===nt.RESOURCE_EXHAUSTED?(mi(e.toString()),mi("Using maximum backoff delay to prevent overloading the backend."),this.t_.Wo()):e&&e.code===nt.UNAUTHENTICATED&&this.state!==3&&(this.authCredentialsProvider.invalidateToken(),this.appCheckCredentialsProvider.invalidateToken()),this.stream!==null&&(this.l_(),this.stream.close(),this.stream=null),this.state=t,await this.listener.mo(e)}l_(){}auth(){this.state=1;const t=this.h_(this.Yo),e=this.Yo;Promise.all([this.authCredentialsProvider.getToken(),this.appCheckCredentialsProvider.getToken()]).then(([n,r])=>{this.Yo===e&&this.P_(n,r)},n=>{t(()=>{const r=new Tt(nt.UNKNOWN,"Fetching auth token failed: "+n.message);return this.I_(r)})})}P_(t,e){const n=this.h_(this.Yo);this.stream=this.T_(t,e),this.stream.Eo(()=>{n(()=>this.listener.Eo())}),this.stream.Ro(()=>{n(()=>(this.state=2,this.Xo=this.ui.enqueueAfterDelay(this.Jo,1e4,()=>(this.r_()&&(this.state=3),Promise.resolve())),this.listener.Ro()))}),this.stream.mo(r=>{n(()=>this.I_(r))}),this.stream.onMessage(r=>{n(()=>++this.e_==1?this.E_(r):this.onNext(r))})}i_(){this.state=5,this.t_.Go(async()=>{this.state=0,this.start()})}I_(t){return At("PersistentStream",`close with error: ${t}`),this.stream=null,this.close(4,t)}h_(t){return e=>{this.ui.enqueueAndForget(()=>this.Yo===t?e():(At("PersistentStream","stream callback skipped by getCloseGuardedDispatcher."),Promise.resolve()))}}}class zw extends Dm{constructor(t,e,n,r,s,o){super(t,"listen_stream_connection_backoff","listen_stream_idle","health_check_timeout",e,n,r,o),this.serializer=s}T_(t,e){return this.connection.Bo("Listen",t,e)}E_(t){return this.onNext(t)}onNext(t){this.t_.reset();const e=tw(this.serializer,t),n=function(s){if(!("targetChange"in s))return Zt.min();const o=s.targetChange;return o.targetIds&&o.targetIds.length?Zt.min():o.readTime?Qn(o.readTime):Zt.min()}(t);return this.listener.d_(e,n)}A_(t){const e={};e.database=Pl(this.serializer),e.addTarget=function(s,o){let a;const c=o.target;if(a=bl(c)?{documents:iw(s,c)}:{query:rw(s,c)._t},a.targetId=o.targetId,o.resumeToken.approximateByteSize()>0){a.resumeToken=Sm(s,o.resumeToken);const h=Rl(s,o.expectedCount);h!==null&&(a.expectedCount=h)}else if(o.snapshotVersion.compareTo(Zt.min())>0){a.readTime=Va(s,o.snapshotVersion.toTimestamp());const h=Rl(s,o.expectedCount);h!==null&&(a.expectedCount=h)}return a}(this.serializer,t);const n=ow(this.serializer,t);n&&(e.labels=n),this.a_(e)}R_(t){const e={};e.database=Pl(this.serializer),e.removeTarget=t,this.a_(e)}}class Gw extends Dm{constructor(t,e,n,r,s,o){super(t,"write_stream_connection_backoff","write_stream_idle","health_check_timeout",e,n,r,o),this.serializer=s}get V_(){return this.e_>0}start(){this.lastStreamToken=void 0,super.start()}l_(){this.V_&&this.m_([])}T_(t,e){return this.connection.Bo("Write",t,e)}E_(t){return ge(!!t.streamToken),this.lastStreamToken=t.streamToken,ge(!t.writeResults||t.writeResults.length===0),this.listener.f_()}onNext(t){ge(!!t.streamToken),this.lastStreamToken=t.streamToken,this.t_.reset();const e=nw(t.writeResults,t.commitTime),n=Qn(t.commitTime);return this.listener.g_(n,e)}p_(){const t={};t.database=Pl(this.serializer),this.a_(t)}m_(t){const e={streamToken:this.lastStreamToken,writes:t.map(n=>ew(this.serializer,n))};this.a_(e)}}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class Ww extends class{}{constructor(t,e,n,r){super(),this.authCredentials=t,this.appCheckCredentials=e,this.connection=n,this.serializer=r,this.y_=!1}w_(){if(this.y_)throw new Tt(nt.FAILED_PRECONDITION,"The client has already been terminated.")}Mo(t,e,n,r){return this.w_(),Promise.all([this.authCredentials.getToken(),this.appCheckCredentials.getToken()]).then(([s,o])=>this.connection.Mo(t,Cl(e,n),r,s,o)).catch(s=>{throw s.name==="FirebaseError"?(s.code===nt.UNAUTHENTICATED&&(this.authCredentials.invalidateToken(),this.appCheckCredentials.invalidateToken()),s):new Tt(nt.UNKNOWN,s.toString())})}Lo(t,e,n,r,s){return this.w_(),Promise.all([this.authCredentials.getToken(),this.appCheckCredentials.getToken()]).then(([o,a])=>this.connection.Lo(t,Cl(e,n),r,o,a,s)).catch(o=>{throw o.name==="FirebaseError"?(o.code===nt.UNAUTHENTICATED&&(this.authCredentials.invalidateToken(),this.appCheckCredentials.invalidateToken()),o):new Tt(nt.UNKNOWN,o.toString())})}terminate(){this.y_=!0,this.connection.terminate()}}class qw{constructor(t,e){this.asyncQueue=t,this.onlineStateHandler=e,this.state="Unknown",this.S_=0,this.b_=null,this.D_=!0}v_(){this.S_===0&&(this.C_("Unknown"),this.b_=this.asyncQueue.enqueueAfterDelay("online_state_timeout",1e4,()=>(this.b_=null,this.F_("Backend didn't respond within 10 seconds."),this.C_("Offline"),Promise.resolve())))}M_(t){this.state==="Online"?this.C_("Unknown"):(this.S_++,this.S_>=1&&(this.x_(),this.F_(`Connection failed 1 times. Most recent error: ${t.toString()}`),this.C_("Offline")))}set(t){this.x_(),this.S_=0,t==="Online"&&(this.D_=!1),this.C_(t)}C_(t){t!==this.state&&(this.state=t,this.onlineStateHandler(t))}F_(t){const e=`Could not reach Cloud Firestore backend. ${t}
This typically indicates that your device does not have a healthy Internet connection at the moment. The client will operate in offline mode until it is able to successfully connect to the backend.`;this.D_?(mi(e),this.D_=!1):At("OnlineStateTracker",e)}x_(){this.b_!==null&&(this.b_.cancel(),this.b_=null)}}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class Xw{constructor(t,e,n,r,s){this.localStore=t,this.datastore=e,this.asyncQueue=n,this.remoteSyncer={},this.O_=[],this.N_=new Map,this.L_=new Set,this.B_=[],this.k_=s,this.k_._o(o=>{n.enqueueAndForget(async()=>{vr(this)&&(At("RemoteStore","Restarting streams for network reachability change."),await async function(c){const h=Jt(c);h.L_.add(4),await Eo(h),h.q_.set("Unknown"),h.L_.delete(4),await nc(h)}(this))})}),this.q_=new qw(n,r)}}async function nc(i){if(vr(i))for(const t of i.B_)await t(!0)}async function Eo(i){for(const t of i.B_)await t(!1)}function Lm(i,t){const e=Jt(i);e.N_.has(t.targetId)||(e.N_.set(t.targetId,t),fh(e)?dh(e):Ms(e).r_()&&uh(e,t))}function hh(i,t){const e=Jt(i),n=Ms(e);e.N_.delete(t),n.r_()&&Nm(e,t),e.N_.size===0&&(n.r_()?n.o_():vr(e)&&e.q_.set("Unknown"))}function uh(i,t){if(i.Q_.xe(t.targetId),t.resumeToken.approximateByteSize()>0||t.snapshotVersion.compareTo(Zt.min())>0){const e=i.remoteSyncer.getRemoteKeysForTarget(t.targetId).size;t=t.withExpectedCount(e)}Ms(i).A_(t)}function Nm(i,t){i.Q_.xe(t),Ms(i).R_(t)}function dh(i){i.Q_=new jM({getRemoteKeysForTarget:t=>i.remoteSyncer.getRemoteKeysForTarget(t),ot:t=>i.N_.get(t)||null,tt:()=>i.datastore.serializer.databaseId}),Ms(i).start(),i.q_.v_()}function fh(i){return vr(i)&&!Ms(i).n_()&&i.N_.size>0}function vr(i){return Jt(i).L_.size===0}function Um(i){i.Q_=void 0}async function jw(i){i.q_.set("Online")}async function Yw(i){i.N_.forEach((t,e)=>{uh(i,t)})}async function $w(i,t){Um(i),fh(i)?(i.q_.M_(t),dh(i)):i.q_.set("Unknown")}async function Kw(i,t,e){if(i.q_.set("Online"),t instanceof xm&&t.state===2&&t.cause)try{await async function(r,s){const o=s.cause;for(const a of s.targetIds)r.N_.has(a)&&(await r.remoteSyncer.rejectListen(a,o),r.N_.delete(a),r.Q_.removeTarget(a))}(i,t)}catch(n){At("RemoteStore","Failed to remove targets %s: %s ",t.targetIds.join(","),n),await Ba(i,n)}else if(t instanceof va?i.Q_.Ke(t):t instanceof Em?i.Q_.He(t):i.Q_.We(t),!e.isEqual(Zt.min()))try{const n=await Im(i.localStore);e.compareTo(n)>=0&&await function(s,o){const a=s.Q_.rt(o);return a.targetChanges.forEach((c,h)=>{if(c.resumeToken.approximateByteSize()>0){const u=s.N_.get(h);u&&s.N_.set(h,u.withResumeToken(c.resumeToken,o))}}),a.targetMismatches.forEach((c,h)=>{const u=s.N_.get(c);if(!u)return;s.N_.set(c,u.withResumeToken(en.EMPTY_BYTE_STRING,u.snapshotVersion)),Nm(s,c);const d=new Pi(u.target,c,h,u.sequenceNumber);uh(s,d)}),s.remoteSyncer.applyRemoteEvent(a)}(i,e)}catch(n){At("RemoteStore","Failed to raise snapshot:",n),await Ba(i,n)}}async function Ba(i,t,e){if(!_o(t))throw t;i.L_.add(1),await Eo(i),i.q_.set("Offline"),e||(e=()=>Im(i.localStore)),i.asyncQueue.enqueueRetryable(async()=>{At("RemoteStore","Retrying IndexedDB access"),await e(),i.L_.delete(1),await nc(i)})}function Om(i,t){return t().catch(e=>Ba(i,e,t))}async function ic(i){const t=Jt(i),e=zi(t);let n=t.O_.length>0?t.O_[t.O_.length-1].batchId:-1;for(;Qw(t);)try{const r=await Nw(t.localStore,n);if(r===null){t.O_.length===0&&e.o_();break}n=r.batchId,Zw(t,r)}catch(r){await Ba(t,r)}Fm(t)&&Vm(t)}function Qw(i){return vr(i)&&i.O_.length<10}function Zw(i,t){i.O_.push(t);const e=zi(i);e.r_()&&e.V_&&e.m_(t.mutations)}function Fm(i){return vr(i)&&!zi(i).n_()&&i.O_.length>0}function Vm(i){zi(i).start()}async function Jw(i){zi(i).p_()}async function tb(i){const t=zi(i);for(const e of i.O_)t.m_(e.mutations)}async function eb(i,t,e){const n=i.O_.shift(),r=rh.from(n,t,e);await Om(i,()=>i.remoteSyncer.applySuccessfulWrite(r)),await ic(i)}async function nb(i,t){t&&zi(i).V_&&await async function(n,r){if(function(o){return WM(o)&&o!==nt.ABORTED}(r.code)){const s=n.O_.shift();zi(n).s_(),await Om(n,()=>n.remoteSyncer.rejectFailedWrite(s.batchId,r)),await ic(n)}}(i,t),Fm(i)&&Vm(i)}async function wf(i,t){const e=Jt(i);e.asyncQueue.verifyOperationInProgress(),At("RemoteStore","RemoteStore received new credentials");const n=vr(e);e.L_.add(3),await Eo(e),n&&e.q_.set("Unknown"),await e.remoteSyncer.handleCredentialChange(t),e.L_.delete(3),await nc(e)}async function ib(i,t){const e=Jt(i);t?(e.L_.delete(2),await nc(e)):t||(e.L_.add(2),await Eo(e),e.q_.set("Unknown"))}function Ms(i){return i.K_||(i.K_=function(e,n,r){const s=Jt(e);return s.w_(),new zw(n,s.connection,s.authCredentials,s.appCheckCredentials,s.serializer,r)}(i.datastore,i.asyncQueue,{Eo:jw.bind(null,i),Ro:Yw.bind(null,i),mo:$w.bind(null,i),d_:Kw.bind(null,i)}),i.B_.push(async t=>{t?(i.K_.s_(),fh(i)?dh(i):i.q_.set("Unknown")):(await i.K_.stop(),Um(i))})),i.K_}function zi(i){return i.U_||(i.U_=function(e,n,r){const s=Jt(e);return s.w_(),new Gw(n,s.connection,s.authCredentials,s.appCheckCredentials,s.serializer,r)}(i.datastore,i.asyncQueue,{Eo:()=>Promise.resolve(),Ro:Jw.bind(null,i),mo:nb.bind(null,i),f_:tb.bind(null,i),g_:eb.bind(null,i)}),i.B_.push(async t=>{t?(i.U_.s_(),await ic(i)):(await i.U_.stop(),i.O_.length>0&&(At("RemoteStore",`Stopping write stream with ${i.O_.length} pending writes`),i.O_=[]))})),i.U_}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class ph{constructor(t,e,n,r,s){this.asyncQueue=t,this.timerId=e,this.targetTimeMs=n,this.op=r,this.removalCallback=s,this.deferred=new Vi,this.then=this.deferred.promise.then.bind(this.deferred.promise),this.deferred.promise.catch(o=>{})}get promise(){return this.deferred.promise}static createAndSchedule(t,e,n,r,s){const o=Date.now()+n,a=new ph(t,e,o,r,s);return a.start(n),a}start(t){this.timerHandle=setTimeout(()=>this.handleDelayElapsed(),t)}skipDelay(){return this.handleDelayElapsed()}cancel(t){this.timerHandle!==null&&(this.clearTimeout(),this.deferred.reject(new Tt(nt.CANCELLED,"Operation cancelled"+(t?": "+t:""))))}handleDelayElapsed(){this.asyncQueue.enqueueAndForget(()=>this.timerHandle!==null?(this.clearTimeout(),this.op().then(t=>this.deferred.resolve(t))):Promise.resolve())}clearTimeout(){this.timerHandle!==null&&(this.removalCallback(this),clearTimeout(this.timerHandle),this.timerHandle=null)}}function mh(i,t){if(mi("AsyncQueue",`${t}: ${i}`),_o(i))return new Tt(nt.UNAVAILABLE,`${t}: ${i}`);throw i}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class is{constructor(t){this.comparator=t?(e,n)=>t(e,n)||Bt.comparator(e.key,n.key):(e,n)=>Bt.comparator(e.key,n.key),this.keyedMap=qs(),this.sortedSet=new Pe(this.comparator)}static emptySet(t){return new is(t.comparator)}has(t){return this.keyedMap.get(t)!=null}get(t){return this.keyedMap.get(t)}first(){return this.sortedSet.minKey()}last(){return this.sortedSet.maxKey()}isEmpty(){return this.sortedSet.isEmpty()}indexOf(t){const e=this.keyedMap.get(t);return e?this.sortedSet.indexOf(e):-1}get size(){return this.sortedSet.size}forEach(t){this.sortedSet.inorderTraversal((e,n)=>(t(e),!1))}add(t){const e=this.delete(t.key);return e.copy(e.keyedMap.insert(t.key,t),e.sortedSet.insert(t,null))}delete(t){const e=this.get(t);return e?this.copy(this.keyedMap.remove(t),this.sortedSet.remove(e)):this}isEqual(t){if(!(t instanceof is)||this.size!==t.size)return!1;const e=this.sortedSet.getIterator(),n=t.sortedSet.getIterator();for(;e.hasNext();){const r=e.getNext().key,s=n.getNext().key;if(!r.isEqual(s))return!1}return!0}toString(){const t=[];return this.forEach(e=>{t.push(e.toString())}),t.length===0?"DocumentSet ()":`DocumentSet (
  `+t.join(`  
`)+`
)`}copy(t,e){const n=new is;return n.comparator=this.comparator,n.keyedMap=t,n.sortedSet=e,n}}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class bf{constructor(){this.W_=new Pe(Bt.comparator)}track(t){const e=t.doc.key,n=this.W_.get(e);n?t.type!==0&&n.type===3?this.W_=this.W_.insert(e,t):t.type===3&&n.type!==1?this.W_=this.W_.insert(e,{type:n.type,doc:t.doc}):t.type===2&&n.type===2?this.W_=this.W_.insert(e,{type:2,doc:t.doc}):t.type===2&&n.type===0?this.W_=this.W_.insert(e,{type:0,doc:t.doc}):t.type===1&&n.type===0?this.W_=this.W_.remove(e):t.type===1&&n.type===2?this.W_=this.W_.insert(e,{type:1,doc:n.doc}):t.type===0&&n.type===1?this.W_=this.W_.insert(e,{type:2,doc:t.doc}):$t():this.W_=this.W_.insert(e,t)}G_(){const t=[];return this.W_.inorderTraversal((e,n)=>{t.push(n)}),t}}class ps{constructor(t,e,n,r,s,o,a,c,h){this.query=t,this.docs=e,this.oldDocs=n,this.docChanges=r,this.mutatedKeys=s,this.fromCache=o,this.syncStateChanged=a,this.excludesMetadataChanges=c,this.hasCachedResults=h}static fromInitialDocuments(t,e,n,r,s){const o=[];return e.forEach(a=>{o.push({type:0,doc:a})}),new ps(t,e,is.emptySet(e),o,n,r,!0,!1,s)}get hasPendingWrites(){return!this.mutatedKeys.isEmpty()}isEqual(t){if(!(this.fromCache===t.fromCache&&this.hasCachedResults===t.hasCachedResults&&this.syncStateChanged===t.syncStateChanged&&this.mutatedKeys.isEqual(t.mutatedKeys)&&Ka(this.query,t.query)&&this.docs.isEqual(t.docs)&&this.oldDocs.isEqual(t.oldDocs)))return!1;const e=this.docChanges,n=t.docChanges;if(e.length!==n.length)return!1;for(let r=0;r<e.length;r++)if(e[r].type!==n[r].type||!e[r].doc.isEqual(n[r].doc))return!1;return!0}}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class rb{constructor(){this.z_=void 0,this.j_=[]}H_(){return this.j_.some(t=>t.J_())}}class sb{constructor(){this.queries=Af(),this.onlineState="Unknown",this.Y_=new Set}terminate(){(function(e,n){const r=Jt(e),s=r.queries;r.queries=Af(),s.forEach((o,a)=>{for(const c of a.j_)c.onError(n)})})(this,new Tt(nt.ABORTED,"Firestore shutting down"))}}function Af(){return new Ts(i=>om(i),Ka)}async function ob(i,t){const e=Jt(i);let n=3;const r=t.query;let s=e.queries.get(r);s?!s.H_()&&t.J_()&&(n=2):(s=new rb,n=t.J_()?0:1);try{switch(n){case 0:s.z_=await e.onListen(r,!0);break;case 1:s.z_=await e.onListen(r,!1);break;case 2:await e.onFirstRemoteStoreListen(r)}}catch(o){const a=mh(o,`Initialization of query '${Yr(t.query)}' failed`);return void t.onError(a)}e.queries.set(r,s),s.j_.push(t),t.Z_(e.onlineState),s.z_&&t.X_(s.z_)&&gh(e)}async function ab(i,t){const e=Jt(i),n=t.query;let r=3;const s=e.queries.get(n);if(s){const o=s.j_.indexOf(t);o>=0&&(s.j_.splice(o,1),s.j_.length===0?r=t.J_()?0:1:!s.H_()&&t.J_()&&(r=2))}switch(r){case 0:return e.queries.delete(n),e.onUnlisten(n,!0);case 1:return e.queries.delete(n),e.onUnlisten(n,!1);case 2:return e.onLastRemoteStoreUnlisten(n);default:return}}function cb(i,t){const e=Jt(i);let n=!1;for(const r of t){const s=r.query,o=e.queries.get(s);if(o){for(const a of o.j_)a.X_(r)&&(n=!0);o.z_=r}}n&&gh(e)}function lb(i,t,e){const n=Jt(i),r=n.queries.get(t);if(r)for(const s of r.j_)s.onError(e);n.queries.delete(t)}function gh(i){i.Y_.forEach(t=>{t.next()})}var Ll,Rf;(Rf=Ll||(Ll={})).ea="default",Rf.Cache="cache";class hb{constructor(t,e,n){this.query=t,this.ta=e,this.na=!1,this.ra=null,this.onlineState="Unknown",this.options=n||{}}X_(t){if(!this.options.includeMetadataChanges){const n=[];for(const r of t.docChanges)r.type!==3&&n.push(r);t=new ps(t.query,t.docs,t.oldDocs,n,t.mutatedKeys,t.fromCache,t.syncStateChanged,!0,t.hasCachedResults)}let e=!1;return this.na?this.ia(t)&&(this.ta.next(t),e=!0):this.sa(t,this.onlineState)&&(this.oa(t),e=!0),this.ra=t,e}onError(t){this.ta.error(t)}Z_(t){this.onlineState=t;let e=!1;return this.ra&&!this.na&&this.sa(this.ra,t)&&(this.oa(this.ra),e=!0),e}sa(t,e){if(!t.fromCache||!this.J_())return!0;const n=e!=="Offline";return(!this.options._a||!n)&&(!t.docs.isEmpty()||t.hasCachedResults||e==="Offline")}ia(t){if(t.docChanges.length>0)return!0;const e=this.ra&&this.ra.hasPendingWrites!==t.hasPendingWrites;return!(!t.syncStateChanged&&!e)&&this.options.includeMetadataChanges===!0}oa(t){t=ps.fromInitialDocuments(t.query,t.docs,t.mutatedKeys,t.fromCache,t.hasCachedResults),this.na=!0,this.ta.next(t)}J_(){return this.options.source!==Ll.Cache}}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class Bm{constructor(t){this.key=t}}class km{constructor(t){this.key=t}}class ub{constructor(t,e){this.query=t,this.Ta=e,this.Ea=null,this.hasCachedResults=!1,this.current=!1,this.da=ce(),this.mutatedKeys=ce(),this.Aa=am(t),this.Ra=new is(this.Aa)}get Va(){return this.Ta}ma(t,e){const n=e?e.fa:new bf,r=e?e.Ra:this.Ra;let s=e?e.mutatedKeys:this.mutatedKeys,o=r,a=!1;const c=this.query.limitType==="F"&&r.size===this.query.limit?r.last():null,h=this.query.limitType==="L"&&r.size===this.query.limit?r.first():null;if(t.inorderTraversal((u,d)=>{const f=r.get(u),p=Qa(this.query,d)?d:null,y=!!f&&this.mutatedKeys.has(f.key),E=!!p&&(p.hasLocalMutations||this.mutatedKeys.has(p.key)&&p.hasCommittedMutations);let _=!1;f&&p?f.data.isEqual(p.data)?y!==E&&(n.track({type:3,doc:p}),_=!0):this.ga(f,p)||(n.track({type:2,doc:p}),_=!0,(c&&this.Aa(p,c)>0||h&&this.Aa(p,h)<0)&&(a=!0)):!f&&p?(n.track({type:0,doc:p}),_=!0):f&&!p&&(n.track({type:1,doc:f}),_=!0,(c||h)&&(a=!0)),_&&(p?(o=o.add(p),s=E?s.add(u):s.delete(u)):(o=o.delete(u),s=s.delete(u)))}),this.query.limit!==null)for(;o.size>this.query.limit;){const u=this.query.limitType==="F"?o.last():o.first();o=o.delete(u.key),s=s.delete(u.key),n.track({type:1,doc:u})}return{Ra:o,fa:n,ns:a,mutatedKeys:s}}ga(t,e){return t.hasLocalMutations&&e.hasCommittedMutations&&!e.hasLocalMutations}applyChanges(t,e,n,r){const s=this.Ra;this.Ra=t.Ra,this.mutatedKeys=t.mutatedKeys;const o=t.fa.G_();o.sort((u,d)=>function(p,y){const E=_=>{switch(_){case 0:return 1;case 2:case 3:return 2;case 1:return 0;default:return $t()}};return E(p)-E(y)}(u.type,d.type)||this.Aa(u.doc,d.doc)),this.pa(n),r=r!=null&&r;const a=e&&!r?this.ya():[],c=this.da.size===0&&this.current&&!r?1:0,h=c!==this.Ea;return this.Ea=c,o.length!==0||h?{snapshot:new ps(this.query,t.Ra,s,o,t.mutatedKeys,c===0,h,!1,!!n&&n.resumeToken.approximateByteSize()>0),wa:a}:{wa:a}}Z_(t){return this.current&&t==="Offline"?(this.current=!1,this.applyChanges({Ra:this.Ra,fa:new bf,mutatedKeys:this.mutatedKeys,ns:!1},!1)):{wa:[]}}Sa(t){return!this.Ta.has(t)&&!!this.Ra.has(t)&&!this.Ra.get(t).hasLocalMutations}pa(t){t&&(t.addedDocuments.forEach(e=>this.Ta=this.Ta.add(e)),t.modifiedDocuments.forEach(e=>{}),t.removedDocuments.forEach(e=>this.Ta=this.Ta.delete(e)),this.current=t.current)}ya(){if(!this.current)return[];const t=this.da;this.da=ce(),this.Ra.forEach(n=>{this.Sa(n.key)&&(this.da=this.da.add(n.key))});const e=[];return t.forEach(n=>{this.da.has(n)||e.push(new km(n))}),this.da.forEach(n=>{t.has(n)||e.push(new Bm(n))}),e}ba(t){this.Ta=t.Ts,this.da=ce();const e=this.ma(t.documents);return this.applyChanges(e,!0)}Da(){return ps.fromInitialDocuments(this.query,this.Ra,this.mutatedKeys,this.Ea===0,this.hasCachedResults)}}class db{constructor(t,e,n){this.query=t,this.targetId=e,this.view=n}}class fb{constructor(t){this.key=t,this.va=!1}}class pb{constructor(t,e,n,r,s,o){this.localStore=t,this.remoteStore=e,this.eventManager=n,this.sharedClientState=r,this.currentUser=s,this.maxConcurrentLimboResolutions=o,this.Ca={},this.Fa=new Ts(a=>om(a),Ka),this.Ma=new Map,this.xa=new Set,this.Oa=new Pe(Bt.comparator),this.Na=new Map,this.La=new ah,this.Ba={},this.ka=new Map,this.qa=fs.kn(),this.onlineState="Unknown",this.Qa=void 0}get isPrimaryClient(){return this.Qa===!0}}async function mb(i,t,e=!0){const n=Xm(i);let r;const s=n.Fa.get(t);return s?(n.sharedClientState.addLocalQueryTarget(s.targetId),r=s.view.Da()):r=await Hm(n,t,e,!0),r}async function gb(i,t){const e=Xm(i);await Hm(e,t,!0,!1)}async function Hm(i,t,e,n){const r=await Uw(i.localStore,Kn(t)),s=r.targetId,o=i.sharedClientState.addLocalQueryTarget(s,e);let a;return n&&(a=await _b(i,t,s,o==="current",r.resumeToken)),i.isPrimaryClient&&e&&Lm(i.remoteStore,r),a}async function _b(i,t,e,n,r){i.Ka=(d,f,p)=>async function(E,_,m,C){let w=_.view.ma(m);w.ns&&(w=await Sf(E.localStore,_.query,!1).then(({documents:T})=>_.view.ma(T,w)));const b=C&&C.targetChanges.get(_.targetId),N=C&&C.targetMismatches.get(_.targetId)!=null,U=_.view.applyChanges(w,E.isPrimaryClient,b,N);return If(E,_.targetId,U.wa),U.snapshot}(i,d,f,p);const s=await Sf(i.localStore,t,!0),o=new ub(t,s.Ts),a=o.ma(s.documents),c=yo.createSynthesizedTargetChangeForCurrentChange(e,n&&i.onlineState!=="Offline",r),h=o.applyChanges(a,i.isPrimaryClient,c);If(i,e,h.wa);const u=new db(t,e,o);return i.Fa.set(t,u),i.Ma.has(e)?i.Ma.get(e).push(t):i.Ma.set(e,[t]),h.snapshot}async function vb(i,t,e){const n=Jt(i),r=n.Fa.get(t),s=n.Ma.get(r.targetId);if(s.length>1)return n.Ma.set(r.targetId,s.filter(o=>!Ka(o,t))),void n.Fa.delete(t);n.isPrimaryClient?(n.sharedClientState.removeLocalQueryTarget(r.targetId),n.sharedClientState.isActiveQueryTarget(r.targetId)||await Dl(n.localStore,r.targetId,!1).then(()=>{n.sharedClientState.clearQueryState(r.targetId),e&&hh(n.remoteStore,r.targetId),Nl(n,r.targetId)}).catch(go)):(Nl(n,r.targetId),await Dl(n.localStore,r.targetId,!0))}async function yb(i,t){const e=Jt(i),n=e.Fa.get(t),r=e.Ma.get(n.targetId);e.isPrimaryClient&&r.length===1&&(e.sharedClientState.removeLocalQueryTarget(n.targetId),hh(e.remoteStore,n.targetId))}async function Eb(i,t,e){const n=Ab(i);try{const r=await function(o,a){const c=Jt(o),h=He.now(),u=a.reduce((p,y)=>p.add(y.key),ce());let d,f;return c.persistence.runTransaction("Locally write mutations","readwrite",p=>{let y=gi(),E=ce();return c.cs.getEntries(p,u).next(_=>{y=_,y.forEach((m,C)=>{C.isValidDocument()||(E=E.add(m))})}).next(()=>c.localDocuments.getOverlayedDocuments(p,y)).next(_=>{d=_;const m=[];for(const C of a){const w=BM(C,d.get(C.key).overlayedDocument);w!=null&&m.push(new _r(C.key,w,Qp(w.value.mapValue),hi.exists(!0)))}return c.mutationQueue.addMutationBatch(p,h,m,a)}).next(_=>{f=_;const m=_.applyToLocalDocumentSet(d,E);return c.documentOverlayCache.saveOverlays(p,_.batchId,m)})}).then(()=>({batchId:f.batchId,changes:lm(d)}))}(n.localStore,t);n.sharedClientState.addPendingMutation(r.batchId),function(o,a,c){let h=o.Ba[o.currentUser.toKey()];h||(h=new Pe(fe)),h=h.insert(a,c),o.Ba[o.currentUser.toKey()]=h}(n,r.batchId,e),await xo(n,r.changes),await ic(n.remoteStore)}catch(r){const s=mh(r,"Failed to persist write");e.reject(s)}}async function zm(i,t){const e=Jt(i);try{const n=await Dw(e.localStore,t);t.targetChanges.forEach((r,s)=>{const o=e.Na.get(s);o&&(ge(r.addedDocuments.size+r.modifiedDocuments.size+r.removedDocuments.size<=1),r.addedDocuments.size>0?o.va=!0:r.modifiedDocuments.size>0?ge(o.va):r.removedDocuments.size>0&&(ge(o.va),o.va=!1))}),await xo(e,n,t)}catch(n){await go(n)}}function Cf(i,t,e){const n=Jt(i);if(n.isPrimaryClient&&e===0||!n.isPrimaryClient&&e===1){const r=[];n.Fa.forEach((s,o)=>{const a=o.view.Z_(t);a.snapshot&&r.push(a.snapshot)}),function(o,a){const c=Jt(o);c.onlineState=a;let h=!1;c.queries.forEach((u,d)=>{for(const f of d.j_)f.Z_(a)&&(h=!0)}),h&&gh(c)}(n.eventManager,t),r.length&&n.Ca.d_(r),n.onlineState=t,n.isPrimaryClient&&n.sharedClientState.setOnlineState(t)}}async function xb(i,t,e){const n=Jt(i);n.sharedClientState.updateQueryState(t,"rejected",e);const r=n.Na.get(t),s=r&&r.key;if(s){let o=new Pe(Bt.comparator);o=o.insert(s,un.newNoDocument(s,Zt.min()));const a=ce().add(s),c=new tc(Zt.min(),new Map,new Pe(fe),o,a);await zm(n,c),n.Oa=n.Oa.remove(s),n.Na.delete(t),_h(n)}else await Dl(n.localStore,t,!1).then(()=>Nl(n,t,e)).catch(go)}async function Sb(i,t){const e=Jt(i),n=t.batch.batchId;try{const r=await Pw(e.localStore,t);Wm(e,n,null),Gm(e,n),e.sharedClientState.updateMutationState(n,"acknowledged"),await xo(e,r)}catch(r){await go(r)}}async function Tb(i,t,e){const n=Jt(i);try{const r=await function(o,a){const c=Jt(o);return c.persistence.runTransaction("Reject batch","readwrite-primary",h=>{let u;return c.mutationQueue.lookupMutationBatch(h,a).next(d=>(ge(d!==null),u=d.keys(),c.mutationQueue.removeMutationBatch(h,d))).next(()=>c.mutationQueue.performConsistencyCheck(h)).next(()=>c.documentOverlayCache.removeOverlaysForBatchId(h,u,a)).next(()=>c.localDocuments.recalculateAndSaveOverlaysForDocumentKeys(h,u)).next(()=>c.localDocuments.getDocuments(h,u))})}(n.localStore,t);Wm(n,t,e),Gm(n,t),n.sharedClientState.updateMutationState(t,"rejected",e),await xo(n,r)}catch(r){await go(r)}}function Gm(i,t){(i.ka.get(t)||[]).forEach(e=>{e.resolve()}),i.ka.delete(t)}function Wm(i,t,e){const n=Jt(i);let r=n.Ba[n.currentUser.toKey()];if(r){const s=r.get(t);s&&(e?s.reject(e):s.resolve(),r=r.remove(t)),n.Ba[n.currentUser.toKey()]=r}}function Nl(i,t,e=null){i.sharedClientState.removeLocalQueryTarget(t);for(const n of i.Ma.get(t))i.Fa.delete(n),e&&i.Ca.$a(n,e);i.Ma.delete(t),i.isPrimaryClient&&i.La.gr(t).forEach(n=>{i.La.containsKey(n)||qm(i,n)})}function qm(i,t){i.xa.delete(t.path.canonicalString());const e=i.Oa.get(t);e!==null&&(hh(i.remoteStore,e),i.Oa=i.Oa.remove(t),i.Na.delete(e),_h(i))}function If(i,t,e){for(const n of e)n instanceof Bm?(i.La.addReference(n.key,t),Mb(i,n)):n instanceof km?(At("SyncEngine","Document no longer in limbo: "+n.key),i.La.removeReference(n.key,t),i.La.containsKey(n.key)||qm(i,n.key)):$t()}function Mb(i,t){const e=t.key,n=e.path.canonicalString();i.Oa.get(e)||i.xa.has(n)||(At("SyncEngine","New document in limbo: "+e),i.xa.add(n),_h(i))}function _h(i){for(;i.xa.size>0&&i.Oa.size<i.maxConcurrentLimboResolutions;){const t=i.xa.values().next().value;i.xa.delete(t);const e=new Bt(be.fromString(t)),n=i.qa.next();i.Na.set(n,new fb(e)),i.Oa=i.Oa.insert(e,n),Lm(i.remoteStore,new Pi(Kn(rm(e.path)),n,"TargetPurposeLimboResolution",Ql.oe))}}async function xo(i,t,e){const n=Jt(i),r=[],s=[],o=[];n.Fa.isEmpty()||(n.Fa.forEach((a,c)=>{o.push(n.Ka(c,t,e).then(h=>{var u;if((h||e)&&n.isPrimaryClient){const d=h?!h.fromCache:(u=e==null?void 0:e.targetChanges.get(c.targetId))===null||u===void 0?void 0:u.current;n.sharedClientState.updateQueryState(c.targetId,d?"current":"not-current")}if(h){r.push(h);const d=lh.Wi(c.targetId,h);s.push(d)}}))}),await Promise.all(o),n.Ca.d_(r),await async function(c,h){const u=Jt(c);try{await u.persistence.runTransaction("notifyLocalViewChanges","readwrite",d=>ot.forEach(h,f=>ot.forEach(f.$i,p=>u.persistence.referenceDelegate.addReference(d,f.targetId,p)).next(()=>ot.forEach(f.Ui,p=>u.persistence.referenceDelegate.removeReference(d,f.targetId,p)))))}catch(d){if(!_o(d))throw d;At("LocalStore","Failed to update sequence numbers: "+d)}for(const d of h){const f=d.targetId;if(!d.fromCache){const p=u.os.get(f),y=p.snapshotVersion,E=p.withLastLimboFreeSnapshotVersion(y);u.os=u.os.insert(f,E)}}}(n.localStore,s))}async function wb(i,t){const e=Jt(i);if(!e.currentUser.isEqual(t)){At("SyncEngine","User change. New user:",t.toKey());const n=await Cm(e.localStore,t);e.currentUser=t,function(s,o){s.ka.forEach(a=>{a.forEach(c=>{c.reject(new Tt(nt.CANCELLED,o))})}),s.ka.clear()}(e,"'waitForPendingWrites' promise is rejected due to a user change."),e.sharedClientState.handleUserChange(t,n.removedBatchIds,n.addedBatchIds),await xo(e,n.hs)}}function bb(i,t){const e=Jt(i),n=e.Na.get(t);if(n&&n.va)return ce().add(n.key);{let r=ce();const s=e.Ma.get(t);if(!s)return r;for(const o of s){const a=e.Fa.get(o);r=r.unionWith(a.view.Va)}return r}}function Xm(i){const t=Jt(i);return t.remoteStore.remoteSyncer.applyRemoteEvent=zm.bind(null,t),t.remoteStore.remoteSyncer.getRemoteKeysForTarget=bb.bind(null,t),t.remoteStore.remoteSyncer.rejectListen=xb.bind(null,t),t.Ca.d_=cb.bind(null,t.eventManager),t.Ca.$a=lb.bind(null,t.eventManager),t}function Ab(i){const t=Jt(i);return t.remoteStore.remoteSyncer.applySuccessfulWrite=Sb.bind(null,t),t.remoteStore.remoteSyncer.rejectFailedWrite=Tb.bind(null,t),t}class ka{constructor(){this.kind="memory",this.synchronizeTabs=!1}async initialize(t){this.serializer=ec(t.databaseInfo.databaseId),this.sharedClientState=this.Wa(t),this.persistence=this.Ga(t),await this.persistence.start(),this.localStore=this.za(t),this.gcScheduler=this.ja(t,this.localStore),this.indexBackfillerScheduler=this.Ha(t,this.localStore)}ja(t,e){return null}Ha(t,e){return null}za(t){return Iw(this.persistence,new Rw,t.initialUser,this.serializer)}Ga(t){return new ww(ch.Zr,this.serializer)}Wa(t){return new Fw}async terminate(){var t,e;(t=this.gcScheduler)===null||t===void 0||t.stop(),(e=this.indexBackfillerScheduler)===null||e===void 0||e.stop(),this.sharedClientState.shutdown(),await this.persistence.shutdown()}}ka.provider={build:()=>new ka};class Ul{async initialize(t,e){this.localStore||(this.localStore=t.localStore,this.sharedClientState=t.sharedClientState,this.datastore=this.createDatastore(e),this.remoteStore=this.createRemoteStore(e),this.eventManager=this.createEventManager(e),this.syncEngine=this.createSyncEngine(e,!t.synchronizeTabs),this.sharedClientState.onlineStateHandler=n=>Cf(this.syncEngine,n,1),this.remoteStore.remoteSyncer.handleCredentialChange=wb.bind(null,this.syncEngine),await ib(this.remoteStore,this.syncEngine.isPrimaryClient))}createEventManager(t){return function(){return new sb}()}createDatastore(t){const e=ec(t.databaseInfo.databaseId),n=function(s){return new Hw(s)}(t.databaseInfo);return function(s,o,a,c){return new Ww(s,o,a,c)}(t.authCredentials,t.appCheckCredentials,n,e)}createRemoteStore(t){return function(n,r,s,o,a){return new Xw(n,r,s,o,a)}(this.localStore,this.datastore,t.asyncQueue,e=>Cf(this.syncEngine,e,0),function(){return Mf.D()?new Mf:new Vw}())}createSyncEngine(t,e){return function(r,s,o,a,c,h,u){const d=new pb(r,s,o,a,c,h);return u&&(d.Qa=!0),d}(this.localStore,this.remoteStore,this.eventManager,this.sharedClientState,t.initialUser,t.maxConcurrentLimboResolutions,e)}async terminate(){var t,e;await async function(r){const s=Jt(r);At("RemoteStore","RemoteStore shutting down."),s.L_.add(5),await Eo(s),s.k_.shutdown(),s.q_.set("Unknown")}(this.remoteStore),(t=this.datastore)===null||t===void 0||t.terminate(),(e=this.eventManager)===null||e===void 0||e.terminate()}}Ul.provider={build:()=>new Ul};/**
 * @license
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *//**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class Rb{constructor(t){this.observer=t,this.muted=!1}next(t){this.muted||this.observer.next&&this.Ya(this.observer.next,t)}error(t){this.muted||(this.observer.error?this.Ya(this.observer.error,t):mi("Uncaught Error in snapshot listener:",t.toString()))}Za(){this.muted=!0}Ya(t,e){setTimeout(()=>{this.muted||t(e)},0)}}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class Cb{constructor(t,e,n,r,s){this.authCredentials=t,this.appCheckCredentials=e,this.asyncQueue=n,this.databaseInfo=r,this.user=hn.UNAUTHENTICATED,this.clientId=Yp.newId(),this.authCredentialListener=()=>Promise.resolve(),this.appCheckCredentialListener=()=>Promise.resolve(),this._uninitializedComponentsProvider=s,this.authCredentials.start(n,async o=>{At("FirestoreClient","Received user=",o.uid),await this.authCredentialListener(o),this.user=o}),this.appCheckCredentials.start(n,o=>(At("FirestoreClient","Received new app check token=",o),this.appCheckCredentialListener(o,this.user)))}get configuration(){return{asyncQueue:this.asyncQueue,databaseInfo:this.databaseInfo,clientId:this.clientId,authCredentials:this.authCredentials,appCheckCredentials:this.appCheckCredentials,initialUser:this.user,maxConcurrentLimboResolutions:100}}setCredentialChangeListener(t){this.authCredentialListener=t}setAppCheckTokenChangeListener(t){this.appCheckCredentialListener=t}terminate(){this.asyncQueue.enterRestrictedMode();const t=new Vi;return this.asyncQueue.enqueueAndForgetEvenWhileRestricted(async()=>{try{this._onlineComponents&&await this._onlineComponents.terminate(),this._offlineComponents&&await this._offlineComponents.terminate(),this.authCredentials.shutdown(),this.appCheckCredentials.shutdown(),t.resolve()}catch(e){const n=mh(e,"Failed to shutdown persistence");t.reject(n)}}),t.promise}}async function nl(i,t){i.asyncQueue.verifyOperationInProgress(),At("FirestoreClient","Initializing OfflineComponentProvider");const e=i.configuration;await t.initialize(e);let n=e.initialUser;i.setCredentialChangeListener(async r=>{n.isEqual(r)||(await Cm(t.localStore,r),n=r)}),t.persistence.setDatabaseDeletedListener(()=>i.terminate()),i._offlineComponents=t}async function Pf(i,t){i.asyncQueue.verifyOperationInProgress();const e=await Ib(i);At("FirestoreClient","Initializing OnlineComponentProvider"),await t.initialize(e,i.configuration),i.setCredentialChangeListener(n=>wf(t.remoteStore,n)),i.setAppCheckTokenChangeListener((n,r)=>wf(t.remoteStore,r)),i._onlineComponents=t}async function Ib(i){if(!i._offlineComponents)if(i._uninitializedComponentsProvider){At("FirestoreClient","Using user provided OfflineComponentProvider");try{await nl(i,i._uninitializedComponentsProvider._offline)}catch(t){const e=t;if(!function(r){return r.name==="FirebaseError"?r.code===nt.FAILED_PRECONDITION||r.code===nt.UNIMPLEMENTED:!(typeof DOMException<"u"&&r instanceof DOMException)||r.code===22||r.code===20||r.code===11}(e))throw e;ls("Error using user provided cache. Falling back to memory cache: "+e),await nl(i,new ka)}}else At("FirestoreClient","Using default OfflineComponentProvider"),await nl(i,new ka);return i._offlineComponents}async function jm(i){return i._onlineComponents||(i._uninitializedComponentsProvider?(At("FirestoreClient","Using user provided OnlineComponentProvider"),await Pf(i,i._uninitializedComponentsProvider._online)):(At("FirestoreClient","Using default OnlineComponentProvider"),await Pf(i,new Ul))),i._onlineComponents}function Pb(i){return jm(i).then(t=>t.syncEngine)}async function Db(i){const t=await jm(i),e=t.eventManager;return e.onListen=mb.bind(null,t.syncEngine),e.onUnlisten=vb.bind(null,t.syncEngine),e.onFirstRemoteStoreListen=gb.bind(null,t.syncEngine),e.onLastRemoteStoreUnlisten=yb.bind(null,t.syncEngine),e}function Lb(i,t,e={}){const n=new Vi;return i.asyncQueue.enqueueAndForget(async()=>function(s,o,a,c,h){const u=new Rb({next:f=>{u.Za(),o.enqueueAndForget(()=>ab(s,d)),f.fromCache&&c.source==="server"?h.reject(new Tt(nt.UNAVAILABLE,'Failed to get documents from server. (However, these documents may exist in the local cache. Run again without setting source to "server" to retrieve the cached documents.)')):h.resolve(f)},error:f=>h.reject(f)}),d=new hb(a,u,{includeMetadataChanges:!0,_a:!0});return ob(s,d)}(await Db(i),i.asyncQueue,t,e,n)),n.promise}/**
 * @license
 * Copyright 2023 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */function Ym(i){const t={};return i.timeoutSeconds!==void 0&&(t.timeoutSeconds=i.timeoutSeconds),t}/**
 * @license
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */const Df=new Map;/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */function $m(i,t,e){if(!e)throw new Tt(nt.INVALID_ARGUMENT,`Function ${i}() cannot be called with an empty ${t}.`)}function Nb(i,t,e,n){if(t===!0&&n===!0)throw new Tt(nt.INVALID_ARGUMENT,`${i} and ${e} cannot be used together.`)}function Lf(i){if(!Bt.isDocumentKey(i))throw new Tt(nt.INVALID_ARGUMENT,`Invalid document reference. Document references must have an even number of segments, but ${i} has ${i.length}.`)}function Nf(i){if(Bt.isDocumentKey(i))throw new Tt(nt.INVALID_ARGUMENT,`Invalid collection reference. Collection references must have an odd number of segments, but ${i} has ${i.length}.`)}function rc(i){if(i===void 0)return"undefined";if(i===null)return"null";if(typeof i=="string")return i.length>20&&(i=`${i.substring(0,20)}...`),JSON.stringify(i);if(typeof i=="number"||typeof i=="boolean")return""+i;if(typeof i=="object"){if(i instanceof Array)return"an array";{const t=function(n){return n.constructor?n.constructor.name:null}(i);return t?`a custom ${t} object`:"an object"}}return typeof i=="function"?"a function":$t()}function Ha(i,t){if("_delegate"in i&&(i=i._delegate),!(i instanceof t)){if(t.name===i.constructor.name)throw new Tt(nt.INVALID_ARGUMENT,"Type does not match the expected instance. Did you pass a reference from a different Firestore SDK?");{const e=rc(i);throw new Tt(nt.INVALID_ARGUMENT,`Expected type '${t.name}', but it was: ${e}`)}}return i}/**
 * @license
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class Uf{constructor(t){var e,n;if(t.host===void 0){if(t.ssl!==void 0)throw new Tt(nt.INVALID_ARGUMENT,"Can't provide ssl option if host option is not set");this.host="firestore.googleapis.com",this.ssl=!0}else this.host=t.host,this.ssl=(e=t.ssl)===null||e===void 0||e;if(this.credentials=t.credentials,this.ignoreUndefinedProperties=!!t.ignoreUndefinedProperties,this.localCache=t.localCache,t.cacheSizeBytes===void 0)this.cacheSizeBytes=41943040;else{if(t.cacheSizeBytes!==-1&&t.cacheSizeBytes<1048576)throw new Tt(nt.INVALID_ARGUMENT,"cacheSizeBytes must be at least 1048576");this.cacheSizeBytes=t.cacheSizeBytes}Nb("experimentalForceLongPolling",t.experimentalForceLongPolling,"experimentalAutoDetectLongPolling",t.experimentalAutoDetectLongPolling),this.experimentalForceLongPolling=!!t.experimentalForceLongPolling,this.experimentalForceLongPolling?this.experimentalAutoDetectLongPolling=!1:t.experimentalAutoDetectLongPolling===void 0?this.experimentalAutoDetectLongPolling=!0:this.experimentalAutoDetectLongPolling=!!t.experimentalAutoDetectLongPolling,this.experimentalLongPollingOptions=Ym((n=t.experimentalLongPollingOptions)!==null&&n!==void 0?n:{}),function(s){if(s.timeoutSeconds!==void 0){if(isNaN(s.timeoutSeconds))throw new Tt(nt.INVALID_ARGUMENT,`invalid long polling timeout: ${s.timeoutSeconds} (must not be NaN)`);if(s.timeoutSeconds<5)throw new Tt(nt.INVALID_ARGUMENT,`invalid long polling timeout: ${s.timeoutSeconds} (minimum allowed value is 5)`);if(s.timeoutSeconds>30)throw new Tt(nt.INVALID_ARGUMENT,`invalid long polling timeout: ${s.timeoutSeconds} (maximum allowed value is 30)`)}}(this.experimentalLongPollingOptions),this.useFetchStreams=!!t.useFetchStreams}isEqual(t){return this.host===t.host&&this.ssl===t.ssl&&this.credentials===t.credentials&&this.cacheSizeBytes===t.cacheSizeBytes&&this.experimentalForceLongPolling===t.experimentalForceLongPolling&&this.experimentalAutoDetectLongPolling===t.experimentalAutoDetectLongPolling&&function(n,r){return n.timeoutSeconds===r.timeoutSeconds}(this.experimentalLongPollingOptions,t.experimentalLongPollingOptions)&&this.ignoreUndefinedProperties===t.ignoreUndefinedProperties&&this.useFetchStreams===t.useFetchStreams}}class sc{constructor(t,e,n,r){this._authCredentials=t,this._appCheckCredentials=e,this._databaseId=n,this._app=r,this.type="firestore-lite",this._persistenceKey="(lite)",this._settings=new Uf({}),this._settingsFrozen=!1,this._terminateTask="notTerminated"}get app(){if(!this._app)throw new Tt(nt.FAILED_PRECONDITION,"Firestore was not initialized using the Firebase SDK. 'app' is not available");return this._app}get _initialized(){return this._settingsFrozen}get _terminated(){return this._terminateTask!=="notTerminated"}_setSettings(t){if(this._settingsFrozen)throw new Tt(nt.FAILED_PRECONDITION,"Firestore has already been started and its settings can no longer be changed. You can only modify settings before calling any other methods on a Firestore object.");this._settings=new Uf(t),t.credentials!==void 0&&(this._authCredentials=function(n){if(!n)return new YT;switch(n.type){case"firstParty":return new ZT(n.sessionIndex||"0",n.iamToken||null,n.authTokenFactory||null);case"provider":return n.client;default:throw new Tt(nt.INVALID_ARGUMENT,"makeAuthCredentialsProvider failed due to invalid credential type")}}(t.credentials))}_getSettings(){return this._settings}_freezeSettings(){return this._settingsFrozen=!0,this._settings}_delete(){return this._terminateTask==="notTerminated"&&(this._terminateTask=this._terminate()),this._terminateTask}async _restart(){this._terminateTask==="notTerminated"?await this._terminate():this._terminateTask="notTerminated"}toJSON(){return{app:this._app,databaseId:this._databaseId,settings:this._settings}}_terminate(){return function(e){const n=Df.get(e);n&&(At("ComponentProvider","Removing Datastore"),Df.delete(e),n.terminate())}(this),Promise.resolve()}}function Ub(i,t,e,n={}){var r;const s=(i=Ha(i,sc))._getSettings(),o=`${t}:${e}`;if(s.host!=="firestore.googleapis.com"&&s.host!==o&&ls("Host has been set in both settings() and connectFirestoreEmulator(), emulator host will be used."),i._setSettings(Object.assign(Object.assign({},s),{host:o,ssl:!1})),n.mockUserToken){let a,c;if(typeof n.mockUserToken=="string")a=n.mockUserToken,c=hn.MOCK_USER;else{a=AS(n.mockUserToken,(r=i._app)===null||r===void 0?void 0:r.options.projectId);const h=n.mockUserToken.sub||n.mockUserToken.user_id;if(!h)throw new Tt(nt.INVALID_ARGUMENT,"mockUserToken must contain 'sub' or 'user_id' field!");c=new hn(h)}i._authCredentials=new $T(new jp(a,c))}}/**
 * @license
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class Gi{constructor(t,e,n){this.converter=e,this._query=n,this.type="query",this.firestore=t}withConverter(t){return new Gi(this.firestore,t,this._query)}}class On{constructor(t,e,n){this.converter=e,this._key=n,this.type="document",this.firestore=t}get _path(){return this._key.path}get id(){return this._key.path.lastSegment()}get path(){return this._key.path.canonicalString()}get parent(){return new Bi(this.firestore,this.converter,this._key.path.popLast())}withConverter(t){return new On(this.firestore,t,this._key)}}class Bi extends Gi{constructor(t,e,n){super(t,e,rm(n)),this._path=n,this.type="collection"}get id(){return this._query.path.lastSegment()}get path(){return this._query.path.canonicalString()}get parent(){const t=this._path.popLast();return t.isEmpty()?null:new On(this.firestore,null,new Bt(t))}withConverter(t){return new Bi(this.firestore,t,this._path)}}function Of(i,t,...e){if(i=cs(i),$m("collection","path",t),i instanceof sc){const n=be.fromString(t,...e);return Nf(n),new Bi(i,null,n)}{if(!(i instanceof On||i instanceof Bi))throw new Tt(nt.INVALID_ARGUMENT,"Expected first argument to collection() to be a CollectionReference, a DocumentReference or FirebaseFirestore");const n=i._path.child(be.fromString(t,...e));return Nf(n),new Bi(i.firestore,null,n)}}function Ob(i,t,...e){if(i=cs(i),arguments.length===1&&(t=Yp.newId()),$m("doc","path",t),i instanceof sc){const n=be.fromString(t,...e);return Lf(n),new On(i,null,new Bt(n))}{if(!(i instanceof On||i instanceof Bi))throw new Tt(nt.INVALID_ARGUMENT,"Expected first argument to collection() to be a CollectionReference, a DocumentReference or FirebaseFirestore");const n=i._path.child(be.fromString(t,...e));return Lf(n),new On(i.firestore,i instanceof Bi?i.converter:null,new Bt(n))}}/**
 * @license
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class Ff{constructor(t=Promise.resolve()){this.Pu=[],this.Iu=!1,this.Tu=[],this.Eu=null,this.du=!1,this.Au=!1,this.Ru=[],this.t_=new Pm(this,"async_queue_retry"),this.Vu=()=>{const n=el();n&&At("AsyncQueue","Visibility state changed to "+n.visibilityState),this.t_.jo()},this.mu=t;const e=el();e&&typeof e.addEventListener=="function"&&e.addEventListener("visibilitychange",this.Vu)}get isShuttingDown(){return this.Iu}enqueueAndForget(t){this.enqueue(t)}enqueueAndForgetEvenWhileRestricted(t){this.fu(),this.gu(t)}enterRestrictedMode(t){if(!this.Iu){this.Iu=!0,this.Au=t||!1;const e=el();e&&typeof e.removeEventListener=="function"&&e.removeEventListener("visibilitychange",this.Vu)}}enqueue(t){if(this.fu(),this.Iu)return new Promise(()=>{});const e=new Vi;return this.gu(()=>this.Iu&&this.Au?Promise.resolve():(t().then(e.resolve,e.reject),e.promise)).then(()=>e.promise)}enqueueRetryable(t){this.enqueueAndForget(()=>(this.Pu.push(t),this.pu()))}async pu(){if(this.Pu.length!==0){try{await this.Pu[0](),this.Pu.shift(),this.t_.reset()}catch(t){if(!_o(t))throw t;At("AsyncQueue","Operation failed with retryable error: "+t)}this.Pu.length>0&&this.t_.Go(()=>this.pu())}}gu(t){const e=this.mu.then(()=>(this.du=!0,t().catch(n=>{this.Eu=n,this.du=!1;const r=function(o){let a=o.message||"";return o.stack&&(a=o.stack.includes(o.message)?o.stack:o.message+`
`+o.stack),a}(n);throw mi("INTERNAL UNHANDLED ERROR: ",r),n}).then(n=>(this.du=!1,n))));return this.mu=e,e}enqueueAfterDelay(t,e,n){this.fu(),this.Ru.indexOf(t)>-1&&(e=0);const r=ph.createAndSchedule(this,t,e,n,s=>this.yu(s));return this.Tu.push(r),r}fu(){this.Eu&&$t()}verifyOperationInProgress(){}async wu(){let t;do t=this.mu,await t;while(t!==this.mu)}Su(t){for(const e of this.Tu)if(e.timerId===t)return!0;return!1}bu(t){return this.wu().then(()=>{this.Tu.sort((e,n)=>e.targetTimeMs-n.targetTimeMs);for(const e of this.Tu)if(e.skipDelay(),t!=="all"&&e.timerId===t)break;return this.wu()})}Du(t){this.Ru.push(t)}yu(t){const e=this.Tu.indexOf(t);this.Tu.splice(e,1)}}class vh extends sc{constructor(t,e,n,r){super(t,e,n,r),this.type="firestore",this._queue=new Ff,this._persistenceKey=(r==null?void 0:r.name)||"[DEFAULT]"}async _terminate(){if(this._firestoreClient){const t=this._firestoreClient.terminate();this._queue=new Ff(t),this._firestoreClient=void 0,await t}}}function Fb(i,t){const e=typeof i=="object"?i:OT(),n=typeof i=="string"?i:"(default)",r=DT(e,"firestore").getImmediate({identifier:n});if(!r._initialized){const s=wS("firestore");s&&Ub(r,...s)}return r}function Km(i){if(i._terminated)throw new Tt(nt.FAILED_PRECONDITION,"The client has already been terminated.");return i._firestoreClient||Vb(i),i._firestoreClient}function Vb(i){var t,e,n;const r=i._freezeSettings(),s=function(a,c,h,u){return new uM(a,c,h,u.host,u.ssl,u.experimentalForceLongPolling,u.experimentalAutoDetectLongPolling,Ym(u.experimentalLongPollingOptions),u.useFetchStreams)}(i._databaseId,((t=i._app)===null||t===void 0?void 0:t.options.appId)||"",i._persistenceKey,r);i._componentsProvider||!((e=r.localCache)===null||e===void 0)&&e._offlineComponentProvider&&(!((n=r.localCache)===null||n===void 0)&&n._onlineComponentProvider)&&(i._componentsProvider={_offline:r.localCache._offlineComponentProvider,_online:r.localCache._onlineComponentProvider}),i._firestoreClient=new Cb(i._authCredentials,i._appCheckCredentials,i._queue,s,i._componentsProvider&&function(a){const c=a==null?void 0:a._online.build();return{_offline:a==null?void 0:a._offline.build(c),_online:c}}(i._componentsProvider))}/**
 * @license
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class ms{constructor(t){this._byteString=t}static fromBase64String(t){try{return new ms(en.fromBase64String(t))}catch(e){throw new Tt(nt.INVALID_ARGUMENT,"Failed to construct data from Base64 string: "+e)}}static fromUint8Array(t){return new ms(en.fromUint8Array(t))}toBase64(){return this._byteString.toBase64()}toUint8Array(){return this._byteString.toUint8Array()}toString(){return"Bytes(base64: "+this.toBase64()+")"}isEqual(t){return this._byteString.isEqual(t._byteString)}}/**
 * @license
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class yh{constructor(...t){for(let e=0;e<t.length;++e)if(t[e].length===0)throw new Tt(nt.INVALID_ARGUMENT,"Invalid field name at argument $(i + 1). Field names must not be empty.");this._internalPath=new Je(t)}isEqual(t){return this._internalPath.isEqual(t._internalPath)}}/**
 * @license
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class Qm{constructor(t){this._methodName=t}}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class Eh{constructor(t,e){if(!isFinite(t)||t<-90||t>90)throw new Tt(nt.INVALID_ARGUMENT,"Latitude must be a number between -90 and 90, but was: "+t);if(!isFinite(e)||e<-180||e>180)throw new Tt(nt.INVALID_ARGUMENT,"Longitude must be a number between -180 and 180, but was: "+e);this._lat=t,this._long=e}get latitude(){return this._lat}get longitude(){return this._long}isEqual(t){return this._lat===t._lat&&this._long===t._long}toJSON(){return{latitude:this._lat,longitude:this._long}}_compareTo(t){return fe(this._lat,t._lat)||fe(this._long,t._long)}}/**
 * @license
 * Copyright 2024 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class xh{constructor(t){this._values=(t||[]).map(e=>e)}toArray(){return this._values.map(t=>t)}isEqual(t){return function(n,r){if(n.length!==r.length)return!1;for(let s=0;s<n.length;++s)if(n[s]!==r[s])return!1;return!0}(this._values,t._values)}}/**
 * @license
 * Copyright 2017 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */const Bb=/^__.*__$/;class kb{constructor(t,e,n){this.data=t,this.fieldMask=e,this.fieldTransforms=n}toMutation(t,e){return this.fieldMask!==null?new _r(t,this.data,this.fieldMask,e,this.fieldTransforms):new vo(t,this.data,e,this.fieldTransforms)}}function Zm(i){switch(i){case 0:case 2:case 1:return!0;case 3:case 4:return!1;default:throw $t()}}class Sh{constructor(t,e,n,r,s,o){this.settings=t,this.databaseId=e,this.serializer=n,this.ignoreUndefinedProperties=r,s===void 0&&this.vu(),this.fieldTransforms=s||[],this.fieldMask=o||[]}get path(){return this.settings.path}get Cu(){return this.settings.Cu}Fu(t){return new Sh(Object.assign(Object.assign({},this.settings),t),this.databaseId,this.serializer,this.ignoreUndefinedProperties,this.fieldTransforms,this.fieldMask)}Mu(t){var e;const n=(e=this.path)===null||e===void 0?void 0:e.child(t),r=this.Fu({path:n,xu:!1});return r.Ou(t),r}Nu(t){var e;const n=(e=this.path)===null||e===void 0?void 0:e.child(t),r=this.Fu({path:n,xu:!1});return r.vu(),r}Lu(t){return this.Fu({path:void 0,xu:!0})}Bu(t){return za(t,this.settings.methodName,this.settings.ku||!1,this.path,this.settings.qu)}contains(t){return this.fieldMask.find(e=>t.isPrefixOf(e))!==void 0||this.fieldTransforms.find(e=>t.isPrefixOf(e.field))!==void 0}vu(){if(this.path)for(let t=0;t<this.path.length;t++)this.Ou(this.path.get(t))}Ou(t){if(t.length===0)throw this.Bu("Document fields must not be empty");if(Zm(this.Cu)&&Bb.test(t))throw this.Bu('Document fields cannot begin and end with "__"')}}class Hb{constructor(t,e,n){this.databaseId=t,this.ignoreUndefinedProperties=e,this.serializer=n||ec(t)}Qu(t,e,n,r=!1){return new Sh({Cu:t,methodName:e,qu:n,path:Je.emptyPath(),xu:!1,ku:r},this.databaseId,this.serializer,this.ignoreUndefinedProperties)}}function Jm(i){const t=i._freezeSettings(),e=ec(i._databaseId);return new Hb(i._databaseId,!!t.ignoreUndefinedProperties,e)}function zb(i,t,e,n,r,s={}){const o=i.Qu(s.merge||s.mergeFields?2:0,t,e,r);ng("Data must be an object, but it was:",o,n);const a=tg(n,o);let c,h;if(s.merge)c=new qn(o.fieldMask),h=o.fieldTransforms;else if(s.mergeFields){const u=[];for(const d of s.mergeFields){const f=Wb(t,d,e);if(!o.contains(f))throw new Tt(nt.INVALID_ARGUMENT,`Field '${f}' is specified in your field mask but missing from your input data.`);Xb(u,f)||u.push(f)}c=new qn(u),h=o.fieldTransforms.filter(d=>c.covers(d.field))}else c=null,h=o.fieldTransforms;return new kb(new Un(a),c,h)}function Gb(i,t,e,n=!1){return Th(e,i.Qu(n?4:3,t))}function Th(i,t){if(eg(i=cs(i)))return ng("Unsupported field value:",t,i),tg(i,t);if(i instanceof Qm)return function(n,r){if(!Zm(r.Cu))throw r.Bu(`${n._methodName}() can only be used with update() and set()`);if(!r.path)throw r.Bu(`${n._methodName}() is not currently supported inside arrays`);const s=n._toFieldTransform(r);s&&r.fieldTransforms.push(s)}(i,t),null;if(i===void 0&&t.ignoreUndefinedProperties)return null;if(t.path&&t.fieldMask.push(t.path),i instanceof Array){if(t.settings.xu&&t.Cu!==4)throw t.Bu("Nested arrays are not supported");return function(n,r){const s=[];let o=0;for(const a of n){let c=Th(a,r.Lu(o));c==null&&(c={nullValue:"NULL_VALUE"}),s.push(c),o++}return{arrayValue:{values:s}}}(i,t)}return function(n,r){if((n=cs(n))===null)return{nullValue:"NULL_VALUE"};if(typeof n=="number")return LM(r.serializer,n);if(typeof n=="boolean")return{booleanValue:n};if(typeof n=="string")return{stringValue:n};if(n instanceof Date){const s=He.fromDate(n);return{timestampValue:Va(r.serializer,s)}}if(n instanceof He){const s=new He(n.seconds,1e3*Math.floor(n.nanoseconds/1e3));return{timestampValue:Va(r.serializer,s)}}if(n instanceof Eh)return{geoPointValue:{latitude:n.latitude,longitude:n.longitude}};if(n instanceof ms)return{bytesValue:Sm(r.serializer,n._byteString)};if(n instanceof On){const s=r.databaseId,o=n.firestore._databaseId;if(!o.isEqual(s))throw r.Bu(`Document reference is for database ${o.projectId}/${o.database} but should be for database ${s.projectId}/${s.database}`);return{referenceValue:oh(n.firestore._databaseId||r.databaseId,n._key.path)}}if(n instanceof xh)return function(o,a){return{mapValue:{fields:{__type__:{stringValue:"__vector__"},value:{arrayValue:{values:o.toArray().map(c=>{if(typeof c!="number")throw a.Bu("VectorValues must only contain numeric values.");return ih(a.serializer,c)})}}}}}}(n,r);throw r.Bu(`Unsupported field value: ${rc(n)}`)}(i,t)}function tg(i,t){const e={};return $p(i)?t.path&&t.path.length>0&&t.fieldMask.push(t.path):xs(i,(n,r)=>{const s=Th(r,t.Mu(n));s!=null&&(e[n]=s)}),{mapValue:{fields:e}}}function eg(i){return!(typeof i!="object"||i===null||i instanceof Array||i instanceof Date||i instanceof He||i instanceof Eh||i instanceof ms||i instanceof On||i instanceof Qm||i instanceof xh)}function ng(i,t,e){if(!eg(e)||!function(r){return typeof r=="object"&&r!==null&&(Object.getPrototypeOf(r)===Object.prototype||Object.getPrototypeOf(r)===null)}(e)){const n=rc(e);throw n==="an object"?t.Bu(i+" a custom object"):t.Bu(i+" "+n)}}function Wb(i,t,e){if((t=cs(t))instanceof yh)return t._internalPath;if(typeof t=="string")return ig(i,t);throw za("Field path arguments must be of type string or ",i,!1,void 0,e)}const qb=new RegExp("[~\\*/\\[\\]]");function ig(i,t,e){if(t.search(qb)>=0)throw za(`Invalid field path (${t}). Paths must not contain '~', '*', '/', '[', or ']'`,i,!1,void 0,e);try{return new yh(...t.split("."))._internalPath}catch{throw za(`Invalid field path (${t}). Paths must not be empty, begin with '.', end with '.', or contain '..'`,i,!1,void 0,e)}}function za(i,t,e,n,r){const s=n&&!n.isEmpty(),o=r!==void 0;let a=`Function ${t}() called with invalid data`;e&&(a+=" (via `toFirestore()`)"),a+=". ";let c="";return(s||o)&&(c+=" (found",s&&(c+=` in field ${n}`),o&&(c+=` in document ${r}`),c+=")"),new Tt(nt.INVALID_ARGUMENT,a+i+c)}function Xb(i,t){return i.some(e=>e.isEqual(t))}/**
 * @license
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class rg{constructor(t,e,n,r,s){this._firestore=t,this._userDataWriter=e,this._key=n,this._document=r,this._converter=s}get id(){return this._key.path.lastSegment()}get ref(){return new On(this._firestore,this._converter,this._key)}exists(){return this._document!==null}data(){if(this._document){if(this._converter){const t=new jb(this._firestore,this._userDataWriter,this._key,this._document,null);return this._converter.fromFirestore(t)}return this._userDataWriter.convertValue(this._document.data.value)}}get(t){if(this._document){const e=this._document.data.field(Mh("DocumentSnapshot.get",t));if(e!==null)return this._userDataWriter.convertValue(e)}}}class jb extends rg{data(){return super.data()}}function Mh(i,t){return typeof t=="string"?ig(i,t):t instanceof yh?t._internalPath:t._delegate._internalPath}/**
 * @license
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */function Yb(i){if(i.limitType==="L"&&i.explicitOrderBy.length===0)throw new Tt(nt.UNIMPLEMENTED,"limitToLast() queries require specifying at least one orderBy() clause")}class wh{}class bh extends wh{}function $b(i,t,...e){let n=[];t instanceof wh&&n.push(t),n=n.concat(e),function(s){const o=s.filter(c=>c instanceof Rh).length,a=s.filter(c=>c instanceof Ah).length;if(o>1||o>0&&a>0)throw new Tt(nt.INVALID_ARGUMENT,"InvalidQuery. When using composite filters, you cannot use more than one filter at the top level. Consider nesting the multiple filters within an `and(...)` statement. For example: change `query(query, where(...), or(...))` to `query(query, and(where(...), or(...)))`.")}(n);for(const r of n)i=r._apply(i);return i}class Ah extends bh{constructor(t,e,n){super(),this._field=t,this._op=e,this._value=n,this.type="where"}static _create(t,e,n){return new Ah(t,e,n)}_apply(t){const e=this._parse(t);return sg(t._query,e),new Gi(t.firestore,t.converter,Al(t._query,e))}_parse(t){const e=Jm(t.firestore);return function(s,o,a,c,h,u,d){let f;if(h.isKeyField()){if(u==="array-contains"||u==="array-contains-any")throw new Tt(nt.INVALID_ARGUMENT,`Invalid Query. You can't perform '${u}' queries on documentId().`);if(u==="in"||u==="not-in"){Bf(d,u);const p=[];for(const y of d)p.push(Vf(c,s,y));f={arrayValue:{values:p}}}else f=Vf(c,s,d)}else u!=="in"&&u!=="not-in"&&u!=="array-contains-any"||Bf(d,u),f=Gb(a,o,d,u==="in"||u==="not-in");return Ve.create(h,u,f)}(t._query,"where",e,t.firestore._databaseId,this._field,this._op,this._value)}}class Rh extends wh{constructor(t,e){super(),this.type=t,this._queryConstraints=e}static _create(t,e){return new Rh(t,e)}_parse(t){const e=this._queryConstraints.map(n=>n._parse(t)).filter(n=>n.getFilters().length>0);return e.length===1?e[0]:Xn.create(e,this._getOperator())}_apply(t){const e=this._parse(t);return e.getFilters().length===0?t:(function(r,s){let o=r;const a=s.getFlattenedFilters();for(const c of a)sg(o,c),o=Al(o,c)}(t._query,e),new Gi(t.firestore,t.converter,Al(t._query,e)))}_getQueryConstraints(){return this._queryConstraints}_getOperator(){return this.type==="and"?"and":"or"}}class Ch extends bh{constructor(t,e){super(),this._field=t,this._direction=e,this.type="orderBy"}static _create(t,e){return new Ch(t,e)}_apply(t){const e=function(r,s,o){if(r.startAt!==null)throw new Tt(nt.INVALID_ARGUMENT,"Invalid query. You must not call startAt() or startAfter() before calling orderBy().");if(r.endAt!==null)throw new Tt(nt.INVALID_ARGUMENT,"Invalid query. You must not call endAt() or endBefore() before calling orderBy().");return new ho(s,o)}(t._query,this._field,this._direction);return new Gi(t.firestore,t.converter,function(r,s){const o=r.explicitOrderBy.concat([s]);return new Ss(r.path,r.collectionGroup,o,r.filters.slice(),r.limit,r.limitType,r.startAt,r.endAt)}(t._query,e))}}function Kb(i,t="asc"){const e=t,n=Mh("orderBy",i);return Ch._create(n,e)}class Ih extends bh{constructor(t,e,n){super(),this.type=t,this._limit=e,this._limitType=n}static _create(t,e,n){return new Ih(t,e,n)}_apply(t){return new Gi(t.firestore,t.converter,Ua(t._query,this._limit,this._limitType))}}function Qb(i){return Ih._create("limit",i,"F")}function Vf(i,t,e){if(typeof(e=cs(e))=="string"){if(e==="")throw new Tt(nt.INVALID_ARGUMENT,"Invalid query. When querying with documentId(), you must provide a valid document ID, but it was an empty string.");if(!sm(t)&&e.indexOf("/")!==-1)throw new Tt(nt.INVALID_ARGUMENT,`Invalid query. When querying a collection by documentId(), you must provide a plain document ID, but '${e}' contains a '/' character.`);const n=t.path.child(be.fromString(e));if(!Bt.isDocumentKey(n))throw new Tt(nt.INVALID_ARGUMENT,`Invalid query. When querying a collection group by documentId(), the value provided must result in a valid document path, but '${n}' is not because it has an odd number of segments (${n.length}).`);return sf(i,new Bt(n))}if(e instanceof On)return sf(i,e._key);throw new Tt(nt.INVALID_ARGUMENT,`Invalid query. When querying with documentId(), you must provide a valid string or a DocumentReference, but it was: ${rc(e)}.`)}function Bf(i,t){if(!Array.isArray(i)||i.length===0)throw new Tt(nt.INVALID_ARGUMENT,`Invalid Query. A non-empty array is required for '${t.toString()}' filters.`)}function sg(i,t){const e=function(r,s){for(const o of r)for(const a of o.getFlattenedFilters())if(s.indexOf(a.op)>=0)return a.op;return null}(i.filters,function(r){switch(r){case"!=":return["!=","not-in"];case"array-contains-any":case"in":return["not-in"];case"not-in":return["array-contains-any","in","not-in","!="];default:return[]}}(t.op));if(e!==null)throw e===t.op?new Tt(nt.INVALID_ARGUMENT,`Invalid query. You cannot use more than one '${t.op.toString()}' filter.`):new Tt(nt.INVALID_ARGUMENT,`Invalid query. You cannot use '${t.op.toString()}' filters with '${e.toString()}' filters.`)}class Zb{convertValue(t,e="none"){switch(mr(t)){case 0:return null;case 1:return t.booleanValue;case 2:return Le(t.integerValue||t.doubleValue);case 3:return this.convertTimestamp(t.timestampValue);case 4:return this.convertServerTimestamp(t,e);case 5:return t.stringValue;case 6:return this.convertBytes(pr(t.bytesValue));case 7:return this.convertReference(t.referenceValue);case 8:return this.convertGeoPoint(t.geoPointValue);case 9:return this.convertArray(t.arrayValue,e);case 11:return this.convertObject(t.mapValue,e);case 10:return this.convertVectorValue(t.mapValue);default:throw $t()}}convertObject(t,e){return this.convertObjectMap(t.fields,e)}convertObjectMap(t,e="none"){const n={};return xs(t,(r,s)=>{n[r]=this.convertValue(s,e)}),n}convertVectorValue(t){var e,n,r;const s=(r=(n=(e=t.fields)===null||e===void 0?void 0:e.value.arrayValue)===null||n===void 0?void 0:n.values)===null||r===void 0?void 0:r.map(o=>Le(o.doubleValue));return new xh(s)}convertGeoPoint(t){return new Eh(Le(t.latitude),Le(t.longitude))}convertArray(t,e){return(t.values||[]).map(n=>this.convertValue(n,e))}convertServerTimestamp(t,e){switch(e){case"previous":const n=Jl(t);return n==null?null:this.convertValue(n,e);case"estimate":return this.convertTimestamp(ao(t));default:return null}}convertTimestamp(t){const e=Hi(t);return new He(e.seconds,e.nanos)}convertDocumentKey(t,e){const n=be.fromString(t);ge(Rm(n));const r=new co(n.get(1),n.get(3)),s=new Bt(n.popFirst(5));return r.isEqual(e)||mi(`Document ${s} contains a document reference within a different database (${r.projectId}/${r.database}) which is not supported. It will be treated as a reference in the current database (${e.projectId}/${e.database}) instead.`),s}}/**
 * @license
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */function Jb(i,t,e){let n;return n=i?i.toFirestore(t):t,n}/**
 * @license
 * Copyright 2020 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */class da{constructor(t,e){this.hasPendingWrites=t,this.fromCache=e}isEqual(t){return this.hasPendingWrites===t.hasPendingWrites&&this.fromCache===t.fromCache}}class tA extends rg{constructor(t,e,n,r,s,o){super(t,e,n,r,o),this._firestore=t,this._firestoreImpl=t,this.metadata=s}exists(){return super.exists()}data(t={}){if(this._document){if(this._converter){const e=new ya(this._firestore,this._userDataWriter,this._key,this._document,this.metadata,null);return this._converter.fromFirestore(e,t)}return this._userDataWriter.convertValue(this._document.data.value,t.serverTimestamps)}}get(t,e={}){if(this._document){const n=this._document.data.field(Mh("DocumentSnapshot.get",t));if(n!==null)return this._userDataWriter.convertValue(n,e.serverTimestamps)}}}class ya extends tA{data(t={}){return super.data(t)}}class eA{constructor(t,e,n,r){this._firestore=t,this._userDataWriter=e,this._snapshot=r,this.metadata=new da(r.hasPendingWrites,r.fromCache),this.query=n}get docs(){const t=[];return this.forEach(e=>t.push(e)),t}get size(){return this._snapshot.docs.size}get empty(){return this.size===0}forEach(t,e){this._snapshot.docs.forEach(n=>{t.call(e,new ya(this._firestore,this._userDataWriter,n.key,n,new da(this._snapshot.mutatedKeys.has(n.key),this._snapshot.fromCache),this.query.converter))})}docChanges(t={}){const e=!!t.includeMetadataChanges;if(e&&this._snapshot.excludesMetadataChanges)throw new Tt(nt.INVALID_ARGUMENT,"To include metadata changes with your document changes, you must also pass { includeMetadataChanges:true } to onSnapshot().");return this._cachedChanges&&this._cachedChangesIncludeMetadataChanges===e||(this._cachedChanges=function(r,s){if(r._snapshot.oldDocs.isEmpty()){let o=0;return r._snapshot.docChanges.map(a=>{const c=new ya(r._firestore,r._userDataWriter,a.doc.key,a.doc,new da(r._snapshot.mutatedKeys.has(a.doc.key),r._snapshot.fromCache),r.query.converter);return a.doc,{type:"added",doc:c,oldIndex:-1,newIndex:o++}})}{let o=r._snapshot.oldDocs;return r._snapshot.docChanges.filter(a=>s||a.type!==3).map(a=>{const c=new ya(r._firestore,r._userDataWriter,a.doc.key,a.doc,new da(r._snapshot.mutatedKeys.has(a.doc.key),r._snapshot.fromCache),r.query.converter);let h=-1,u=-1;return a.type!==0&&(h=o.indexOf(a.doc.key),o=o.delete(a.doc.key)),a.type!==1&&(o=o.add(a.doc),u=o.indexOf(a.doc.key)),{type:nA(a.type),doc:c,oldIndex:h,newIndex:u}})}}(this,e),this._cachedChangesIncludeMetadataChanges=e),this._cachedChanges}}function nA(i){switch(i){case 0:return"added";case 2:case 3:return"modified";case 1:return"removed";default:return $t()}}class iA extends Zb{constructor(t){super(),this.firestore=t}convertBytes(t){return new ms(t)}convertReference(t){const e=this.convertDocumentKey(t,this.firestore._databaseId);return new On(this.firestore,null,e)}}function rA(i){i=Ha(i,Gi);const t=Ha(i.firestore,vh),e=Km(t),n=new iA(t);return Yb(i._query),Lb(e,i._query).then(r=>new eA(t,n,i,r))}function sA(i,t){const e=Ha(i.firestore,vh),n=Ob(i),r=Jb(i.converter,t);return oA(e,[zb(Jm(i.firestore),"addDoc",n._key,r,i.converter!==null,{}).toMutation(n._key,hi.exists(!1))]).then(()=>n)}function oA(i,t){return function(n,r){const s=new Vi;return n.asyncQueue.enqueueAndForget(async()=>Eb(await Pb(n),r,s)),s.promise}(Km(i),t)}(function(t,e=!0){(function(r){Es=r})(UT),Da(new ro("firestore",(n,{instanceIdentifier:r,options:s})=>{const o=n.getProvider("app").getImmediate(),a=new vh(new KT(n.getProvider("auth-internal")),new tM(n.getProvider("app-check-internal")),function(h,u){if(!Object.prototype.hasOwnProperty.apply(h.options,["projectId"]))throw new Tt(nt.INVALID_ARGUMENT,'"projectId" not provided in firebase.initializeApp.');return new co(h.options.projectId,u)}(o,r),o);return s=Object.assign({useFetchStreams:e},s),a._setSettings(s),a},"PUBLIC").setMultipleInstances(!0)),ns(Jd,"4.7.3",t),ns(Jd,"4.7.3","esm2017")})();const og={apiKey:"AIzaSyDHXZSUgnamqv2FlfwDgGO_REaW29imNL8",authDomain:"drift-b0eef.firebaseapp.com",projectId:"drift-b0eef",storageBucket:"drift-b0eef.firebasestorage.app",messagingSenderId:"1029587762443",appId:"1:1029587762443:web:c15719f3917b143be8b60e"},Ea=og.apiKey!=="YOUR_API_KEY";let kf=null,Qr=null;if(Ea)try{kf=Vp(og),Qr=Fb(kf),console.log("Firebase initialized successfully")}catch(i){console.warn("Firebase initialization failed:",i)}else console.log("Firebase not configured - using local scores only");function aA(i){let t=5381,e=52711;for(let s=0;s<i.length;s++){const o=i.charCodeAt(s);t=(t<<5)+t^o,e=(e<<5)+e^o}const n=(t>>>0).toString(16).padStart(8,"0"),r=(e>>>0).toString(16).padStart(8,"0");return n+r}const cA=new Set(["4eaboref55a49d06","4dcf87e555891e88","4dd007e5558c9e88","4dd087e5558d1e88","4da58eef5576a64c","4daf0f7f5580293c","4da60eef5576c64c","4db00f7f55804d3c","4e1d8f63559f2a1e","4e1c0f63559caa1e","4e1e0f6355a02a1e","4c3083c553414460","4c2f03c5533ec460","4c3103c55341c460","4dd90ead5595a1e8","4dd80ead5593a1e8","4dda0ead559621e8","4c4983c1534d4420","4c4803c1534ac420","4c4a03c1534dc420","4d9c83d9556844f4","4d9f03d9556d44f4","4d9e03d9556ac4f4","4da003d9556ec4f4","4d9d83d95569c4f4","4e1c0eef559cc66c","4e1c8f23559ce492","4c3a03fd534e0538","4c3a83fd534e8538","4e5e0f0b55e44b46","4e5f0f0b55e5cb46","4dd38ff1558d0f8c","4dd40ff1558e8f8c","4e640eef55eb466c","4e5f8ead55e62928","4e5e0ead55e3a928","4de70f8155a02c3c","4dd48f4d558f0d28","4dd50f4d55908d28","4e4d8ec7559cd1a2","8c710715a9eb0e30","8c700715a9e88e30","4c4e03e9535250a4","4c4f03e9535350a4","4c4f83e9535450a4","4c4e83e9535150a4","4d9a8725556584d0","4d9c0725556804d0","4d9c872555688cd0","4d9a072555640cd0","4dfc0eff55993b5a","4dfb8eff5598bb5a","4dfd0eff559abb5a","4df78ebf55942778","4df88ebf5595a778","4df80f0f5595aaea","4df90f0f55972aea","4dd90f7155959f0c","4dd98f71559620ec","4e678f0355eecbbe","4df58f0d5591ab68","4e088f1b559b6af6","4e5d0eb755e3070e","4e5d8eb755e4870e","4e5e8eb755e5070e","4e5c0eb755e0870e","4e640eb755ebc70e","4e018eb555929d2a","4e000eb555900d2a","4e660eb155eec6ce","4e658eb155ed46ce","4c3403f1533dc4f0","4c3383f1533c44f0","4dfe8f0b559b4b86","4dff0f0b559ccb86","4dff8f0b559e4b86","4e518f6b55e0e606","4e520f6b55e26606","4e4f0f6b55dc6606","4dac8f0b557d4946","4dad0f0b557ec946","4dae0f0b55804946","4e640ec755ebc6ee","4e4b8eb355d7c7ce","4e548ea955e5072e","4de58f6d559d8b64","4de48f6d559c0b64","4de68f6d559f0b64","4e0f8f2555aa8b12","4e748f1955ff8b26","4e4d0ebb55dd87a6","4e4c0ebb55db87a6","4e4d8ebb55de87a6","4e0d8f6155a7e902","4e0e0f6155a96902","4e0e8f6155aae902","4c2b03e1533644e0","4c2a83e1533544e0","4c2b83e1533744e0","4e428f0755ce8b3e","4e6e0f1955f80ae6","4e6e8f1955f98ae6","4e6f0f1955fb0ae6","4dfc8f6f5599bc2a","4dfd0f6f559b3c2a","4dfd8f6f559cbc2a","4e528ef755e34532","4e530ef755e4c532","4e538ef755e64532","4deb0f71559425cc","4dec0f71559625cc","4dec8f7155972664","4e520eb755e2c70e","4e548f1b55e4ab36","4e540f1b55e32b36","4e558f1b55e62b36","4e170f7f55b32a5c","4e168f7f55b1aa5c","4e178f7f55b4aa5c","4c2e83d55339c4b4","4c2e03d5533844b4","4c2f03d5533a44b4","4e0f8f0b55aa4b06","4e638eef55e9c66c","4e0d0f1555a6ea56","4e4e8ec155dd5122","4e788f0355058c3e","4da48f17557564d2","4e018f6555934582","4e000f6555910582","4dfe8f65558fc502","4dd08fe7558946a2","4dd10fe7558ac6a2","4dd18fe7558c46a2","4e028f65559545c2","4e650f0b55ec4b86","4e658f0b55edcb86","4e008f65559105c2","4e548f6b55e4e646","4e4f8f6b55dde606"]);function ag(i){if(!i||typeof i!="string")return!1;const t=i.toUpperCase().replace(/[^A-Z]/g,"");if(t.length!==3)return!1;const e=aA(t);return!cA.has(e)}function Hf(){return"AAA"}function lA(i){if(!i||typeof i!="string")return Hf();const t=i.toUpperCase().replace(/[^A-Z]/g,"").slice(0,3).padEnd(3,"A");return ag(t)?t:Hf()}const zf="drift-high-scores-v2",il=5,hA=10;class js{constructor(){this.score=0,this.combo=1,this.maxCombo=1,this.comboTimer=0,this.comboDecayTime=3,this.highScores=this.loadHighScores(),this.globalLeaderboards={},this.globalLeaderboardsLoading={},this.onScoreChange=null,this.onComboChange=null,this.onNewHighScore=null}reset(){this.score=0,this.combo=1,this.maxCombo=1,this.comboTimer=0}addPoints(t){const e=Math.floor(t*this.combo);return this.score+=e,this.comboTimer=this.comboDecayTime,this.onScoreChange&&this.onScoreChange(this.score,e),e}addRawPoints(t){return this.score+=t,this.onScoreChange&&this.onScoreChange(this.score,t),t}increaseCombo(){this.combo+=1,this.comboTimer=this.comboDecayTime,this.combo>this.maxCombo&&(this.maxCombo=this.combo),this.onComboChange&&this.onComboChange(this.combo)}resetCombo(){this.combo>1&&(this.combo=1,this.onComboChange&&this.onComboChange(this.combo))}update(t){this.combo>1&&(this.comboTimer-=t,this.comboTimer<=0&&this.resetCombo())}getComboTimeRemaining(){return Math.max(0,this.comboTimer/this.comboDecayTime)}getScoreInfo(){return{score:this.score,combo:this.combo,maxCombo:this.maxCombo,comboTimeRemaining:this.getComboTimeRemaining()}}finalizeScore(t){const e=this.getLeaderboard(t),n=this.getRankForScore(t,this.score),r=n<=il&&this.score>0;return{finalScore:this.score,maxCombo:this.maxCombo,isHighScore:r,rank:r?n:null,topScore:e.length>0?e[0].score:0}}getRankForScore(t,e){const n=this.getLeaderboard(t);let r=1;for(const s of n){if(e>s.score)break;r++}return r}isTopScore(t,e){return this.getRankForScore(t,e)<=il&&e>0}validateInitials(t){return ag(t)}addHighScore(t,e,n){const r=lA(n);this.highScores[t]||(this.highScores[t]=[]);const s={initials:r,score:e,date:Date.now()};return this.highScores[t].push(s),this.highScores[t].sort((o,a)=>a.score-o.score),this.highScores[t]=this.highScores[t].slice(0,il),this.saveHighScores(),this.submitToGlobalLeaderboard(t,e,r),this.onNewHighScore&&this.onNewHighScore(t,e,r),this.getRankForScore(t,e)}async submitToGlobalLeaderboard(t,e,n){if(!Ea||!Qr)return console.log("Firebase not configured - score saved locally only"),!1;try{const r=Of(Qr,"leaderboards",t,"scores");return await sA(r,{initials:n,score:e,date:Date.now()}),console.log("Score submitted to global leaderboard"),this.fetchGlobalLeaderboard(t),!0}catch(r){return console.warn("Failed to submit score to Firebase:",r),!1}}async fetchGlobalLeaderboard(t){if(!Ea||!Qr)return[];if(this.globalLeaderboardsLoading[t])return this.globalLeaderboards[t]||[];this.globalLeaderboardsLoading[t]=!0;try{const e=Of(Qr,"leaderboards",t,"scores"),n=$b(e,Kb("score","desc"),Qb(hA)),r=await rA(n),s=[];return r.forEach(o=>{s.push(o.data())}),this.globalLeaderboards[t]=s,s}catch(e){return console.warn("Failed to fetch global leaderboard:",e),[]}finally{this.globalLeaderboardsLoading[t]=!1}}getGlobalLeaderboard(t){return this.globalLeaderboards[t]||this.fetchGlobalLeaderboard(t),this.globalLeaderboards[t]||[]}isGlobalLeaderboardAvailable(){return Ea&&Qr!==null}getLeaderboard(t){return this.highScores[t]||[]}getHighScore(t){const e=this.getLeaderboard(t);return e.length>0?e[0].score:0}getAllHighScores(){return{...this.highScores}}loadHighScores(){try{const t=localStorage.getItem(zf);if(t)return JSON.parse(t)}catch(t){console.warn("Failed to load high scores:",t)}return{}}saveHighScores(){try{localStorage.setItem(zf,JSON.stringify(this.highScores))}catch(t){console.warn("Failed to save high scores:",t)}}clearHighScores(){this.highScores={},this.saveHighScores()}}class uA{constructor(t,e){this.water=t,this.terrain=e}getWaterInCircle(t,e,n){let r=0;const s=n*n;for(let o=-n;o<=n;o++)for(let a=-n;a<=n;a++){if(a*a+o*o>s)continue;const h=Math.floor(t+a),u=Math.floor(e+o);this.water.inBounds(h,u)&&(r+=this.water.getDepth(h,u))}return r}getWaterInRect(t,e,n,r){let s=0;const o=Math.floor(Math.min(t,n)),a=Math.floor(Math.max(t,n)),c=Math.floor(Math.min(e,r)),h=Math.floor(Math.max(e,r));for(let u=c;u<=h;u++)for(let d=o;d<=a;d++)this.water.inBounds(d,u)&&(s+=this.water.getDepth(d,u));return s}isCircleFilled(t,e,n,r){return this.getWaterInCircle(t,e,n)>=r}isRectFilled(t,e,n,r,s){return this.getWaterInRect(t,e,n,r)>=s}getAverageDepthInCircle(t,e,n){let r=0,s=0;const o=n*n;for(let a=-n;a<=n;a++)for(let c=-n;c<=n;c++){if(c*c+a*a>o)continue;const u=Math.floor(t+c),d=Math.floor(e+a);this.water.inBounds(u,d)&&(r+=this.water.getDepth(u,d),s++)}return s>0?r/s:0}hasWater(t,e,n,r=.5){const s=n*n;for(let o=-n;o<=n;o++)for(let a=-n;a<=n;a++){if(a*a+o*o>s)continue;const h=Math.floor(t+a),u=Math.floor(e+o);if(this.water.inBounds(h,u)&&this.water.getDepth(h,u)>=r)return!0}return!1}getFlowInCircle(t,e,n){let r=0,s=0,o=0;const a=n*n;for(let d=-n;d<=n;d++)for(let f=-n;f<=n;f++){if(f*f+d*d>a)continue;const y=Math.floor(t+f),E=Math.floor(e+d);if(this.water.inBounds(y,E)){const _=E*this.water.width+y;this.water.depth[_]>.1&&(r+=this.water.velocityX[_],s+=this.water.velocityY[_],o++)}}if(o===0)return{vx:0,vy:0,speed:0};const c=r/o,h=s/o,u=Math.sqrt(c*c+h*h);return{vx:c,vy:h,speed:u}}getTerrainHeight(t,e){return this.terrain.getHeight(Math.floor(t),Math.floor(e))}findRandomPosition(t=20,e=-50,n=1/0){for(let a=0;a<50;a++){const c=t+Math.random()*(this.terrain.width-t*2),h=t+Math.random()*(this.terrain.height-t*2),u=this.terrain.getHeight(Math.floor(c),Math.floor(h));if(u>=e&&u<=n)return{x:c,y:h,height:u}}const s=this.terrain.width/2,o=this.terrain.height/2;return{x:s,y:o,height:this.terrain.getHeight(s,o)}}}class Ph{constructor({terrain:t,water:e,scene:n,manager:r}){this.terrain=t,this.water=e,this.scene=n,this.manager=r,this.zoneDetector=new uA(e,t),this.scoreManager=new js,this.gameOver=!1,this.victory=!1,this.elapsedTime=0,this.lives=3,this.maxLives=3,this.visualElements=[]}init(){this.scoreManager.reset(),this.gameOver=!1,this.victory=!1,this.elapsedTime=0,this.lives=this.maxLives,this.terrain.generate(),this.water.reset()}update(t){this.elapsedTime+=t,this.scoreManager.update(t)}cleanup(){for(const t of this.visualElements)this.scene.remove(t);this.visualElements=[]}getScore(){return this.scoreManager.score}getStats(){return{score:this.scoreManager.score,maxCombo:this.scoreManager.maxCombo,elapsedTime:this.elapsedTime,lives:this.lives}}isGameOver(){return this.gameOver}isVictory(){return this.victory}loseLife(){return this.lives--,this.lives<=0&&(this.gameOver=!0,this.victory=!1),this.lives}addPoints(t){const e=this.scoreManager.addPoints(t);return this.manager.reportScoreUpdate(this.scoreManager.score,this.scoreManager.combo),e}increaseCombo(){this.scoreManager.increaseCombo(),this.manager.reportScoreUpdate(this.scoreManager.score,this.scoreManager.combo)}getDisplayInfo(){return{score:this.scoreManager.score,combo:this.scoreManager.combo,comboTimer:this.scoreManager.getComboTimeRemaining(),lives:this.lives,maxLives:this.maxLives,elapsedTime:this.elapsedTime}}addVisualElement(t){return this.scene.add(t),this.visualElements.push(t),t}removeVisualElement(t){this.scene.remove(t);const e=this.visualElements.indexOf(t);e!==-1&&this.visualElements.splice(e,1)}}const Ji={ACTIVE:"active",FILLING:"filling",COMPLETE:"complete",EXPIRED:"expired"};class dA extends Ph{constructor(t){super(t),this.gameTime=120,this.maxTargets=3,this.targetLifetime=15,this.targetRadius=6,this.fillThreshold=50,this.spawnDelay=2,this.targets=[],this.spawnTimer=0,this.completedTargets=0,this.basePoints=100,this.timeBonus=50,this.maxHearts=3,this.quarterHearts=12,this.maxQuarterHearts=12}init(){super.init(),this.targets=[],this.spawnTimer=0,this.completedTargets=0,this.quarterHearts=this.maxQuarterHearts;for(let t=0;t<2;t++)this.spawnTarget();console.log("Flow Rush started! Guide water to the glowing targets.")}update(t){if(super.update(t),this.gameTime-this.elapsedTime<=0){this.gameOver=!0,this.victory=!0;return}this.spawnTimer+=t,this.spawnTimer>=this.spawnDelay&&this.targets.length<this.maxTargets&&(this.spawnTarget(),this.spawnTimer=0);for(let n=this.targets.length-1;n>=0;n--){const r=this.targets[n];this.updateTarget(r,t),(r.state===Ji.COMPLETE||r.state===Ji.EXPIRED)&&(this.removeTarget(r),this.targets.splice(n,1))}}spawnTarget(){const t=this.findValidTargetPosition();if(!t)return null;const e={x:t.x,y:t.y,height:t.height,radius:this.targetRadius,state:Ji.ACTIVE,lifetime:this.targetLifetime,timeRemaining:this.targetLifetime,fillAmount:0,fillThreshold:this.fillThreshold,mesh:null,ring:null,pulsePhase:Math.random()*Math.PI*2};return this.createTargetVisual(e),this.targets.push(e),e}findValidTargetPosition(){const e=this.targetRadius*3,n=30;for(let r=0;r<n;r++){const s=this.zoneDetector.findRandomPosition(20,-20,30);let o=!0;for(const a of this.targets){const c=s.x-a.x,h=s.y-a.y;if(Math.sqrt(c*c+h*h)<e){o=!1;break}}if(o)return s}return null}gridToWorld(t,e){const n=Ye();return{x:(t-this.terrain.width/2)*n,z:(e-this.terrain.height/2)*n}}createTargetVisual(t){const e=Ye();t.radius*e;const n=48,s=this.createTerrainFollowingRing(t.x,t.y,t.radius,1.5,n),o=new bn({color:16763904,transparent:!0,opacity:.9,side:_n}),a=new Ie(s,o),c=this.createTerrainFollowingDisc(t.x,t.y,t.radius*.9,n),h=new bn({color:16772744,transparent:!0,opacity:.25,side:_n}),u=new Ie(c,h),d=new rr,f=8,p=new ja(.5,1,f,8),y=new bn({color:16768256,transparent:!0,opacity:.8});for(let E=0;E<4;E++){const _=E/4*Math.PI*2,m=t.x+Math.cos(_)*t.radius,C=t.y+Math.sin(_)*t.radius,w=this.zoneDetector.getTerrainHeight(m,C),b=this.gridToWorld(m,C),N=w+f/2,U=new Ie(p,y.clone());U.position.set(b.x,N,b.z),d.add(U)}t.ring=a,t.fill=u,t.beacons=d,t.material=o,t.fillMaterial=h,t.beaconMaterial=y,t.worldScale=e,this.addVisualElement(a),this.addVisualElement(u),this.addVisualElement(d)}createTerrainFollowingRing(t,e,n,r,s){const o=n-r/2,a=n+r/2,c=new yn,h=[],u=[];for(let d=0;d<=s;d++){const f=d/s*Math.PI*2,p=Math.cos(f),y=Math.sin(f),E=t+p*o,_=e+y*o,m=t+p*a,C=e+y*a,w=this.zoneDetector.getTerrainHeight(E,_),b=this.zoneDetector.getTerrainHeight(m,C),N=this.gridToWorld(E,_),U=this.gridToWorld(m,C);if(h.push(N.x,w+.5,N.z),h.push(U.x,b+.5,U.z),d<s){const T=d*2;u.push(T,T+1,T+2),u.push(T+1,T+3,T+2)}}return c.setAttribute("position",new Re(h,3)),c.setIndex(u),c.computeVertexNormals(),c}updateTargetGeometry(t){if(t.ring&&t.ring.geometry){const r=t.ring.geometry.attributes.position.array,s=t.radius-1.5/2,o=t.radius+1.5/2;for(let a=0;a<=48;a++){const c=a/48*Math.PI*2,h=Math.cos(c),u=Math.sin(c),d=t.x+h*s,f=t.y+u*s,p=t.x+h*o,y=t.y+u*o,E=this.zoneDetector.getTerrainHeight(d,f),_=this.zoneDetector.getTerrainHeight(p,y);this.gridToWorld(d,f),this.gridToWorld(p,y);const m=a*6;r[m+1]=E+.5,r[m+4]=_+.5}t.ring.geometry.attributes.position.needsUpdate=!0}if(t.fill&&t.fill.geometry){const r=t.fill.geometry.attributes.position.array,s=t.radius*.9,o=this.zoneDetector.getTerrainHeight(t.x,t.y);r[1]=o+.3;for(let a=0;a<=48;a++){const c=a/48*Math.PI*2,h=t.x+Math.cos(c)*s,u=t.y+Math.sin(c)*s,d=this.zoneDetector.getTerrainHeight(h,u),f=(a+1)*3;r[f+1]=d+.3}t.fill.geometry.attributes.position.needsUpdate=!0}t.beacons&&t.beacons.children.forEach((r,s)=>{const o=s/4*Math.PI*2,a=t.x+Math.cos(o)*t.radius,c=t.y+Math.sin(o)*t.radius;this.zoneDetector.getTerrainHeight(a,c);const h=this.gridToWorld(a,c);r.position.x=h.x,r.position.z=h.z})}createTerrainFollowingDisc(t,e,n,r){const s=new yn,o=[],a=[],c=this.zoneDetector.getTerrainHeight(t,e),h=this.gridToWorld(t,e);o.push(h.x,c+.3,h.z);for(let u=0;u<=r;u++){const d=u/r*Math.PI*2,f=t+Math.cos(d)*n,p=e+Math.sin(d)*n,y=this.zoneDetector.getTerrainHeight(f,p),E=this.gridToWorld(f,p);o.push(E.x,y+.3,E.z),u<r&&a.push(0,u+1,u+2)}return s.setAttribute("position",new Re(o,3)),s.setIndex(a),s.computeVertexNormals(),s}updateTarget(t,e){t.timeRemaining-=e;const n=this.zoneDetector.getWaterInCircle(t.x,t.y,t.radius);if(n>0&&(t.fillAmount+=n*e*.5,t.state=Ji.FILLING),t.fillAmount>=t.fillThreshold){this.onTargetComplete(t);return}if(t.timeRemaining<=0){this.onTargetExpired(t);return}this.updateTargetVisual(t,e)}updateTargetVisual(t,e){if(!t.ring||!t.material)return;this.updateTargetGeometry(t),t.pulsePhase+=e*4;const n=.5+.5*Math.sin(t.pulsePhase),r=t.timeRemaining/t.lifetime,s=t.fillAmount/t.fillThreshold;if(r<.3){const o=Math.sin(t.pulsePhase*4)>0;t.material.color.setHex(o?16724787:16737792),t.beacons&&t.beacons.children.forEach(a=>{a.material.color.setHex(o?16724787:16737792)})}else t.state===Ji.FILLING?(t.material.color.setHex(6750207),t.beacons&&t.beacons.children.forEach(o=>{o.material.color.setHex(6750207)})):(t.material.color.setHex(16763904),t.beacons&&t.beacons.children.forEach(o=>{o.material.color.setHex(16768256)}));if(t.material.opacity=.6+n*.4,t.fill&&(t.fillMaterial.opacity=.15+s*.4),t.beacons){const o=Math.sin(t.pulsePhase*1.5)*2;t.beacons.children.forEach((a,c)=>{const h=c/4*Math.PI*2,u=t.x+Math.cos(h)*t.radius,d=t.y+Math.sin(h)*t.radius,p=this.zoneDetector.getTerrainHeight(u,d)+4;a.position.y=p+o+Math.sin(t.pulsePhase+c*1.5)*1,a.material.opacity=.5+n*.5})}}onTargetComplete(t){t.state=Ji.COMPLETE;const e=Math.floor(t.timeRemaining*this.timeBonus),n=this.basePoints+e;this.addPoints(n),this.increaseCombo(),this.completedTargets++,console.log(`Target complete! +${n} points (${e} time bonus)`)}onTargetExpired(t){t.state=Ji.EXPIRED,this.quarterHearts--,this.scoreManager.resetCombo(),this.quarterHearts<=0&&(this.gameOver=!0,this.victory=!1),console.log(`Target expired - lost a quarter heart! (${this.quarterHearts}/${this.maxQuarterHearts} remaining)`)}removeTarget(t){t.ring&&(this.removeVisualElement(t.ring),t.ring.geometry.dispose(),t.material.dispose()),t.fill&&(this.removeVisualElement(t.fill),t.fill.geometry.dispose(),t.fillMaterial.dispose()),t.beacons&&(this.removeVisualElement(t.beacons),t.beacons.children.forEach(e=>{e.geometry.dispose(),e.material.dispose()}))}cleanup(){for(const t of this.targets)this.removeTarget(t);this.targets=[],super.cleanup()}getDisplayInfo(){const t=super.getDisplayInfo();return t.timeRemaining=Math.max(0,this.gameTime-this.elapsedTime),t.targetsCompleted=this.completedTargets,t.activeTargets=this.targets.length,t.quarterHearts=this.quarterHearts,t.maxQuarterHearts=this.maxQuarterHearts,t}getStats(){const t=super.getStats();return t.targetsCompleted=this.completedTargets,t.timeRemaining=Math.max(0,this.gameTime-this.elapsedTime),t}}const In={WAITING:"waiting",READY:"ready",TRIGGERED:"triggered",COMPLETED:"completed"};class fA extends Ph{constructor(t){super(t),this.gameTime=120,this.numRings=5,this.ringRadius=4,this.ringVisualRadius=3,this.cascadeLifetime=15,this.rings=[],this.currentRingIndex=0,this.cascadesCompleted=0,this.cascadeTimeRemaining=0,this.basePoints=50,this.sequenceBonus=25,this.cascadeCompleteBonus=500,this.flowCooldown=0,this.ringResetDelay=2,this.isRegenerating=!1,this.maxHearts=3,this.quarterHearts=12,this.maxQuarterHearts=12}init(){super.init(),this.rings=[],this.currentRingIndex=0,this.cascadesCompleted=0,this.flowCooldown=0,this.isRegenerating=!1,this.quarterHearts=this.maxQuarterHearts,this.generateCascade(),console.log("Cascade started! Guide water through the rings in order.")}generateCascade(t=0){this.isRegenerating=!0;for(const a of this.rings)this.removeRing(a);this.rings=[],this.currentRingIndex=0;const e=this.findHighPoint();let n=e.x,r=e.y,s=e.height,o={x:0,y:1};for(let a=0;a<this.numRings;a++){const c=this.findDownhillPosition(n,r,s,o);if(!c){if(t>=3){console.warn(`Could only place ${a} rings after ${t} retries, using partial cascade`);break}console.warn(`Could only place ${a} rings, retrying...`),setTimeout(()=>this.generateCascade(t+1),300);return}const h={index:a,x:c.x,y:c.y,height:c.height,radius:this.ringRadius,state:a===0?In.READY:In.WAITING,flowDirection:c.direction,triggered:!1,triggerTime:0,mesh:null,glowMesh:null};this.createRingVisual(h),this.rings.push(h),n=c.x,r=c.y,s=c.height,o=c.direction}this.cascadeTimeRemaining=this.cascadeLifetime,this.isRegenerating=!1,console.log(`Cascade generated with ${this.rings.length} rings`)}getTerrainHeightPercentile(t){const e=[];for(let s=0;s<this.terrain.height;s+=8)for(let o=0;o<this.terrain.width;o+=8)e.push(this.zoneDetector.getTerrainHeight(o,s));e.sort((s,o)=>s-o);const r=Math.floor(e.length*t);return e[Math.min(r,e.length-1)]}findHighPoint(){const e=this.getTerrainHeightPercentile(.6);let n=null,r=-1/0;for(let s=0;s<50;s++){const o=25+Math.random()*(this.terrain.width-50),a=25+Math.random()*(this.terrain.height-25*2),c=this.zoneDetector.getTerrainHeight(o,a);if(c<e)continue;const h=this.water.getDepth(Math.floor(o),Math.floor(a));if(h>.5)continue;let u=c;u-=h*50,u>r&&(r=u,n={x:o,y:a,height:c})}if(!n){const s=this.terrain.width/2,o=this.terrain.height/4;n={x:s,y:o,height:this.zoneDetector.getTerrainHeight(s,o)}}return n}findDownhillPosition(t,e,n,r){const s=12+Math.random()*8,o=40,a=20;let c=null,h=-1/0;const u=Math.atan2(r.y,r.x);for(let d=0;d<o;d++){const f=(Math.random()-.5)*Math.PI*.6,p=u+f,y=t+Math.cos(p)*s,E=e+Math.sin(p)*s;if(y<a||y>this.terrain.width-a||E<a||E>this.terrain.height-a)continue;const _=this.zoneDetector.getTerrainHeight(y,E),m=n-_;if(m<2)continue;const C=this.water.getDepth(Math.floor(y),Math.floor(E));let w=10;w+=Math.min(m,30),w-=Math.max(0,m-40)*.5,w-=Math.abs(f)*2,w-=C*10,w>h&&(h=w,c={x:y,y:E,height:_,direction:{x:Math.cos(p),y:Math.sin(p)}})}if(!c)for(let d=0;d<20;d++){const f=Math.random()*Math.PI*2,p=s*.8,y=t+Math.cos(f)*p,E=e+Math.sin(f)*p;if(y<a||y>this.terrain.width-a||E<a||E>this.terrain.height-a)continue;const _=this.zoneDetector.getTerrainHeight(y,E);if(n-_>-10){c={x:y,y:E,height:_,direction:{x:Math.cos(f),y:Math.sin(f)}};break}}return c}gridToWorld(t,e){const n=Ye();return{x:(t-this.terrain.width/2)*n,z:(e-this.terrain.height/2)*n}}createRingVisual(t){const e=Ye(),n=this.gridToWorld(t.x,t.y),r=this.ringVisualRadius*e,s=new Ya(r,r*.15,8,24),o=new bn({color:t.state===In.READY?4521864:8947848,transparent:!0,opacity:.8,side:_n}),a=new Ie(s,o),c=Math.atan2(t.flowDirection.y,t.flowDirection.x);a.rotation.y=-c+Math.PI/2,a.rotation.x=Math.PI/2;const h=this.zoneDetector.getTerrainHeight(t.x,t.y);a.position.set(n.x,h+r*1.2,n.z);const u=document.createElement("canvas");u.width=64,u.height=64;const d=u.getContext("2d");d.fillStyle="#ffffff",d.font="bold 48px Arial",d.textAlign="center",d.textBaseline="middle",d.fillText(String(t.index+1),32,32);const f=new Lx(u),p=new Tp({map:f,transparent:!0}),y=new Dx(p);y.scale.set(4,4,1),y.position.set(n.x,h+r*2.5,n.z),t.mesh=a,t.numberSprite=y,t.material=o,t.worldPos=n,t.ringWorldRadius=r,this.addVisualElement(a),this.addVisualElement(y)}update(t){if(super.update(t),this.gameTime-this.elapsedTime<=0){this.gameOver=!0,this.victory=!0;return}this.flowCooldown>0&&(this.flowCooldown-=t),this.updateCascadeTimer(t),this.checkRingFlow(t);for(const n of this.rings)this.updateRingVisual(n,t)}updateCascadeTimer(t){this.isRegenerating||this.rings.length!==0&&(this.cascadeTimeRemaining<=0||(this.cascadeTimeRemaining-=t,this.cascadeTimeRemaining<=0&&(this.cascadeTimeRemaining=0,this.onCascadeExpired())))}onCascadeExpired(){if(this.isRegenerating)return;this.isRegenerating=!0;const t=this.rings.length-this.currentRingIndex,e=Math.max(0,t-1);this.quarterHearts-=e,this.scoreManager.resetCombo(),e>0?console.log(`Cascade expired! Missed ${t} rings. Lost ${e} quarter hearts. (${this.quarterHearts}/${this.maxQuarterHearts})`):console.log("Cascade expired! Only missed 1 ring - no penalty.");for(let n=this.currentRingIndex;n<this.rings.length;n++)this.rings[n]&&(this.rings[n].state=In.COMPLETED);if(this.quarterHearts<=0){this.quarterHearts=0,this.gameOver=!0,this.victory=!1;return}setTimeout(()=>{this.gameOver||this.generateCascade()},this.ringResetDelay*1e3)}checkRingFlow(t){if(this.flowCooldown>0||this.isRegenerating||this.rings.length===0||this.currentRingIndex>=this.rings.length)return;const e=this.rings[this.currentRingIndex];if(!e||e.state!==In.READY)return;const n=this.zoneDetector.getFlowInCircle(e.x,e.y,e.radius);this.zoneDetector.hasWater(e.x,e.y,e.radius,.3)&&n.speed>.3&&this.onRingTriggered(e)}onRingTriggered(t){t.state=In.TRIGGERED,t.triggerTime=.5;const e=this.basePoints+this.sequenceBonus*t.index;if(this.addPoints(e),this.increaseCombo(),console.log(`Ring ${t.index+1} triggered! +${e} points`),setTimeout(()=>{t.state===In.TRIGGERED&&(t.state=In.COMPLETED)},500),this.currentRingIndex++,this.flowCooldown=.5,this.currentRingIndex>=this.rings.length)this.onCascadeComplete();else{const n=this.rings[this.currentRingIndex];n&&(n.state=In.READY)}}onCascadeComplete(){this.cascadesCompleted++,this.addPoints(this.cascadeCompleteBonus),this.isRegenerating=!0,this.cascadeTimeRemaining=0,console.log(`CASCADE COMPLETE! +${this.cascadeCompleteBonus} bonus!`),setTimeout(()=>{this.gameOver||this.generateCascade()},this.ringResetDelay*1e3)}updateRingVisual(t,e){if(!t.mesh||!t.material)return;const n=this.zoneDetector.getTerrainHeight(t.x,t.y);if(t.mesh.position.y=n+t.ringWorldRadius*1.2,t.numberSprite&&(t.numberSprite.position.y=n+t.ringWorldRadius*2.5),t.triggerTime>0){t.triggerTime-=e,t.material.color.setHex(16777215),t.material.opacity=.5+Math.sin(t.triggerTime*20)*.5;return}switch(t.state){case In.READY:if(this.cascadeTimeRemaining/this.cascadeLifetime<.3){const o=Math.sin(this.elapsedTime*10)>0;t.material.color.setHex(o?16724787:16737792),t.material.opacity=.8+Math.sin(this.elapsedTime*8)*.2}else t.material.color.setHex(4521864),t.material.opacity=.7+Math.sin(this.elapsedTime*4)*.2;break;case In.WAITING:if(this.cascadeTimeRemaining/this.cascadeLifetime<.3){const o=Math.sin(this.elapsedTime*10)>0;t.material.color.setHex(o?11149858:8930304),t.material.opacity=.5}else t.material.color.setHex(6710886),t.material.opacity=.4;break;case In.COMPLETED:t.material.color.setHex(4500223),t.material.opacity=.3;break}}removeRing(t){t.mesh&&(this.removeVisualElement(t.mesh),t.mesh.geometry.dispose(),t.material.dispose()),t.numberSprite&&(this.removeVisualElement(t.numberSprite),t.numberSprite.material.map.dispose(),t.numberSprite.material.dispose())}cleanup(){for(const t of this.rings)this.removeRing(t);this.rings=[],super.cleanup()}getDisplayInfo(){const t=super.getDisplayInfo();return t.timeRemaining=Math.max(0,this.gameTime-this.elapsedTime),t.cascadesCompleted=this.cascadesCompleted,t.currentRing=this.currentRingIndex+1,t.totalRings=this.numRings,t.quarterHearts=this.quarterHearts,t.maxQuarterHearts=this.maxQuarterHearts,t.cascadeTimeRemaining=Math.max(0,this.cascadeTimeRemaining),t.cascadeLifetime=this.cascadeLifetime,t}getStats(){const t=super.getStats();return t.cascadesCompleted=this.cascadesCompleted,t}}class pA extends Ph{constructor(t){super(t),this.gameTime=120,this.villageRadius=5,this.villageVisualRadius=3,this.numStartingSprings=3,this.springFlowRate=800,this.springs=[],this.numStartingVillages=5,this.villagesPerSpawn=2,this.villageSpawnInterval=15,this.villageSpawnTimer=15,this.maxVillages=20,this.floodThreshold=.5,this.floodDamageInterval=3,this.floodDestroyTime=12,this.villages=[],this.villagesSaved=0,this.villagesLost=0,this.villageSavePoints=200,this.villageSurviveTime=30,this.survivalPoints=5,this.maxHearts=3,this.quarterHearts=12,this.maxQuarterHearts=12}init(){super.init(),this.villages=[],this.springs=[],this.villageSpawnTimer=this.villageSpawnInterval,this.villagesSaved=0,this.villagesLost=0,this.quarterHearts=this.maxQuarterHearts,this.generateSprings(this.numStartingSprings);for(let t=0;t<this.numStartingVillages;t++)this.spawnVillage();console.log(`Flood Defense started! Protect ${this.villages.length} villages from the floods!`)}generateSprings(t){for(let e=0;e<t;e++)this.addSpring();console.log(`Generated ${t} springs at highest points (${this.springs.length} total)`)}addSpring(){try{const t=this.findHighestSpringPosition();if(!t)return console.warn("Could not find position for spring"),null;const e=Math.floor(t.x),n=Math.floor(t.y);this.water.addSpring(e,n,this.springFlowRate);const r={x:e,y:n,height:t.height};return this.springs.push(r),r}catch(t){return console.warn("Error adding spring:",t),null}}findHighestSpringPosition(){if(!this.terrain||!this.terrain.width||!this.terrain.height)return null;const r=[],s=this.terrain.width,o=this.terrain.height;try{for(let a=20;a<o-20;a+=8)for(let c=20;c<s-20;c+=8){const h=this.zoneDetector.getTerrainHeight(c,a);typeof h=="number"&&!isNaN(h)&&r.push({x:c,y:a,height:h})}}catch(a){return console.warn("Error sampling terrain for springs:",a),null}if(r.length===0)return null;r.sort((a,c)=>c.height-a.height);for(const a of r){let c=!1;for(const h of this.springs)if(Math.sqrt((a.x-h.x)**2+(a.y-h.y)**2)<25){c=!0;break}if(!c)return a}return r[0]}getTerrainHeightPercentile(t){const e=[];for(let s=0;s<this.terrain.height;s+=8)for(let o=0;o<this.terrain.width;o+=8)e.push(this.zoneDetector.getTerrainHeight(o,s));e.sort((s,o)=>s-o);const r=Math.floor(e.length*t);return e[Math.min(r,e.length-1)]}spawnVillage(){if(this.villages.length>=this.maxVillages)return;const t=this.getTerrainHeightPercentile(.4),e=this.findVillagePosition(t);if(!e){console.warn("Could not find position for new village");return}const n={index:this.villagesSaved+this.villagesLost+this.villages.length,x:e.x,y:e.y,height:e.height,radius:this.villageRadius,isFlooded:!1,floodTime:0,lastDamageTick:0,aliveTime:0,mesh:null,buildings:[]};this.createVillageVisual(n),this.villages.push(n),console.log(`New village spawned! (${this.villages.length} active)`)}findVillagePosition(t){let s=null,o=-1/0;for(let a=0;a<50;a++){const c=20+Math.random()*(this.terrain.width-40),h=20+Math.random()*(this.terrain.height-20*2),u=this.zoneDetector.getTerrainHeight(c,h);if(u>t)continue;const d=this.water.getDepth(Math.floor(c),Math.floor(h));if(d>.3)continue;let f=!1;for(const y of this.villages)if(Math.sqrt((c-y.x)**2+(h-y.y)**2)<25){f=!0;break}if(f)continue;for(const y of this.springs)if(Math.sqrt((c-y.x)**2+(h-y.y)**2)<35){f=!0;break}if(f)continue;let p=100-d*50;p-=u*.5,p>o&&(o=p,s={x:c,y:h,height:u})}return s}gridToWorld(t,e){const n=Ye();return{x:(t-this.terrain.width/2)*n,z:(e-this.terrain.height/2)*n}}createVillageVisual(t){const e=Ye(),n=this.gridToWorld(t.x,t.y),r=this.zoneDetector.getTerrainHeight(t.x,t.y),s=[],o=[{x:0,z:0,size:1.2,height:2.5},{x:1.5,z:.8,size:.8,height:1.8},{x:-1,z:1.2,size:.9,height:2},{x:.5,z:-1.2,size:.7,height:1.5}];for(const E of o){const _=new _s(E.size,E.height,E.size),m=new bn({color:14535833,transparent:!0,opacity:.9}),C=new Ie(_,m);C.position.set(n.x+E.x*e*.4,r+E.height/2,n.z+E.z*e*.4),s.push(C),this.addVisualElement(C)}const a=this.villageRadius*e,c=12,h=new Ya(a,.4,8,32),u=new bn({color:4521864,transparent:!0,opacity:.7}),d=new Ie(h,u);d.rotation.x=Math.PI/2,d.position.set(n.x,r+c,n.z);const f=new rr,p=new ja(.4,.8,c,8),y=new bn({color:4521864,transparent:!0,opacity:.8});for(let E=0;E<4;E++){const _=E/4*Math.PI*2,m=t.x+Math.cos(_)*t.radius,C=t.y+Math.sin(_)*t.radius,w=this.zoneDetector.getTerrainHeight(m,C),b=this.gridToWorld(m,C),N=new Ie(p,y.clone());N.position.set(b.x,w+c/2,b.z),f.add(N)}t.buildings=s,t.ring=d,t.ringMaterial=u,t.buildingMaterials=s.map(E=>E.material),t.beacons=f,t.beaconGeom=p,t.beaconMat=y,t.beaconHeight=c,t.worldPos=n,t.worldScale=e,this.addVisualElement(d),this.addVisualElement(f)}update(t){if(this.gameOver)return;if(super.update(t),this.gameTime-this.elapsedTime<=0){this.onGameEnd();return}if(!(this.quarterHearts<=0)){if(Math.floor(this.elapsedTime)>Math.floor(this.elapsedTime-t)&&(this.scoreManager.addRawPoints(this.survivalPoints),this.manager.reportScoreUpdate(this.scoreManager.score,this.scoreManager.combo)),this.villageSpawnTimer-=t,this.villageSpawnTimer<=0){for(let n=0;n<this.villagesPerSpawn;n++)this.spawnVillage();this.addSpring(),console.log(`Wave! Added ${this.villagesPerSpawn} villages and 1 spring. Total springs: ${this.springs.length}`),this.villageSpawnTimer=this.villageSpawnInterval}this.updateVillages(t)}}updateVillages(t){if(this.gameOver)return;const e=[];for(const n of this.villages){if(this.gameOver)break;if(n.aliveTime+=t,!n.saved&&n.aliveTime>=this.villageSurviveTime&&(n.saved=!0,this.villagesSaved++,this.addPoints(this.villageSavePoints),this.increaseCombo(),console.log(`Village saved! +${this.villageSavePoints} points`)),this.getAverageWaterDepth(n.x,n.y,n.radius)>this.floodThreshold){n.isFlooded=!0,n.floodTime+=t;const s=Math.floor(n.floodTime/this.floodDamageInterval);if(s>n.lastDamageTick){const o=s-n.lastDamageTick;n.lastDamageTick=s;for(let a=0;a<o&&(this.onFloodDamage(n),!this.gameOver);a++);}!this.gameOver&&n.floodTime>=this.floodDestroyTime&&(e.push(n),this.onVillageDestroyed(n))}else n.isFlooded=!1,n.floodTime=Math.max(0,n.floodTime-t*.5),n.lastDamageTick=Math.floor(n.floodTime/this.floodDamageInterval);this.gameOver||this.updateVillageVisual(n,t)}if(!this.gameOver){for(const n of e){this.removeVillage(n);const r=this.villages.indexOf(n);r!==-1&&this.villages.splice(r,1)}for(const n of e)this.spawnVillage()}}getAverageWaterDepth(t,e,n){let r=0,s=0;for(let o=-n;o<=n;o+=2)for(let a=-n;a<=n;a+=2){if(a*a+o*o>n*n)continue;const c=Math.floor(t+a),h=Math.floor(e+o);c>=0&&c<this.terrain.width&&h>=0&&h<this.terrain.height&&(r+=this.water.getDepth(c,h),s++)}return s>0?r/s:0}onFloodDamage(t){if(this.gameOver||this.quarterHearts<=0)return;if(this.quarterHearts--,this.quarterHearts<=0){this.quarterHearts=0,this.onGameEnd();return}try{this.scoreManager.resetCombo()}catch{}const e=t?t.lastDamageTick:0;console.log(`Village flooding! (${e*3}s) Lost 1 quarter heart. (${this.quarterHearts}/${this.maxQuarterHearts})`)}onVillageDestroyed(t){this.gameOver||(this.villagesLost++,console.log(`Village destroyed after 12s flooding! Relocating survivors... (Lost: ${this.villagesLost})`))}onGameEnd(){if(!this.gameOver){this.gameOver=!0,this.victory=this.quarterHearts>0;try{const t=[...this.villages];for(const e of t)if(e&&!e.saved){this.villagesSaved++;const n=Math.min(1,(e.aliveTime||0)/this.villageSurviveTime),r=Math.floor(this.villageSavePoints*n);r>0&&this.scoreManager.addRawPoints(r)}this.manager&&this.manager.reportScoreUpdate&&this.manager.reportScoreUpdate(this.scoreManager.score,this.scoreManager.combo),console.log(`Game ended! Saved ${this.villagesSaved} villages, lost ${this.villagesLost}. Final score: ${this.scoreManager.score}`)}catch(t){console.warn("Error in onGameEnd:",t)}}}updateVillageVisual(t,e){if(!(!t||!t.ring))try{const n=this.zoneDetector.getTerrainHeight(t.x,t.y)||0,r=t.beaconHeight||12,s=Math.sin(this.elapsedTime*1.5)*1;let o=n;for(let p=0;p<4;p++){const y=p/4*Math.PI*2,E=t.x+Math.cos(y)*t.radius,_=t.y+Math.sin(y)*t.radius,m=this.zoneDetector.getTerrainHeight(E,_);typeof m=="number"&&m>o&&(o=m)}t.ring&&(t.ring.position.y=o+r+2+s);const a=[2.5,1.8,2,1.5];for(let p=0;p<t.buildings.length;p++){const y=t.buildings[p],E=a[p]||2;y.position.y=n+E/2}t.beacons&&t.beacons.children.forEach((p,y)=>{const E=y/4*Math.PI*2,_=t.x+Math.cos(E)*t.radius,m=t.y+Math.sin(E)*t.radius,C=this.zoneDetector.getTerrainHeight(_,m),w=this.gridToWorld(_,m);p.position.x=w.x,p.position.z=w.z,p.position.y=C+r/2+s});let c,h,u,d=.7,f=.8;if(t.isFlooded){const p=t.floodTime/this.floodDestroyTime,y=8+p*12,E=Math.sin(this.elapsedTime*y)>0;c=E?16724787:16737792,h=E?16724787:16737792,u=E?16746632:16755336,d=.9,f=.9;for(const _ of t.buildingMaterials)_.color.setHex(u),_.opacity=.9-p*.4}else if(t.floodTime>0){c=16763972,h=16763972,u=14535833,d=.7;for(const p of t.buildingMaterials)p.color.setHex(u),p.opacity=.9}else if(t.saved){c=16768324,h=16768324,u=15654297,d=.8;for(const p of t.buildingMaterials)p.color.setHex(u)}else{const p=.6+Math.sin(this.elapsedTime*2)*.1;c=4521864,h=4521864,u=14535833,d=p,f=p+.2;for(const y of t.buildingMaterials)y.color.setHex(u),y.opacity=.9}t.ringMaterial.color.setHex(c),t.ringMaterial.opacity=d,t.beacons&&t.beacons.children.forEach(p=>{p.material.color.setHex(h),p.material.opacity=f})}catch(n){console.warn("Village visual update error:",n)}}removeVillage(t){if(t)try{if(t.ring&&(this.removeVisualElement(t.ring),t.ring.geometry&&t.ring.geometry.dispose(),t.ringMaterial&&t.ringMaterial.dispose(),t.ring=null),t.beacons&&(this.removeVisualElement(t.beacons),t.beacons.children.forEach(e=>{e.material&&e.material.dispose()}),t.beacons=null),t.beaconGeom&&(t.beaconGeom.dispose(),t.beaconGeom=null),t.buildings){for(const e of t.buildings)this.removeVisualElement(e),e.geometry&&e.geometry.dispose(),e.material&&e.material.dispose();t.buildings=[]}}catch(e){console.warn("Error removing village:",e)}}cleanup(){this.gameOver=!0;try{const t=[...this.villages];this.villages=[];for(const e of t)this.removeVillage(e)}catch(t){console.warn("Error cleaning up villages:",t)}try{const t=[...this.springs];this.springs=[];for(const e of t)try{e&&typeof e.x=="number"&&typeof e.y=="number"&&this.water.removeSpringNear(e.x,e.y,1)}catch(n){console.warn("Error removing spring:",n)}}catch(t){console.warn("Error cleaning up springs:",t)}try{super.cleanup()}catch(t){console.warn("Error in base cleanup:",t)}}getDisplayInfo(){const t=super.getDisplayInfo();return t.timeRemaining=Math.max(0,this.gameTime-this.elapsedTime),t.villagesSaved=this.villagesSaved,t.villagesLost=this.villagesLost,t.activeVillages=this.villages.length,t.quarterHearts=this.quarterHearts,t.maxQuarterHearts=this.maxQuarterHearts,t.nextVillageIn=Math.max(0,this.villageSpawnTimer),t}getStats(){const t=super.getStats();return t.villagesSaved=this.villagesSaved,t.villagesLost=this.villagesLost,t}}function mA(){const i=kx();if(i)return console.log("Loaded saved quality:",i),i;const t=zx();return jl(t),console.log("Auto-detected quality:",t),t}mA();const gA=Ip();class Gf{constructor(){this.inputMode=gA,this.container=document.getElementById("app"),this.scene=new Jx(this.container),this.terrain=new Bd,this.water=new kd(this.terrain),this.terrainMesh=new eS(this.terrain),this.waterMesh=new sS(this.water,this.terrain),this.springMarkers=new oS(this.water,this.terrain),this.scene.add(this.terrainMesh.object),this.scene.add(this.waterMesh.object),this.scene.add(this.springMarkers.object),this.gameModeManager=new mS(this.terrain,this.water,this.scene.scene),this.gameModeManager.registerMode(ci.FLOW_RUSH,dA),this.gameModeManager.registerMode(ci.CASCADE,fA),this.gameModeManager.registerMode(ci.FLOOD_DEFENSE,pA),this.setupGameModeCallbacks(),this.inputMode===ln.TOUCH?(this.controls=new cS(this.scene,this.terrain,this.terrainMesh),this.setupMobileControlCallbacks(),document.body.classList.add("touch-mode")):(this.controls=new aS(this.scene,this.terrain,this.terrainMesh),this.setupDesktopControlCallbacks(),document.body.classList.add("desktop-mode"));const t=this.terrain.reset.bind(this.terrain);this.terrain.reset=()=>{t(),this.water.reset(),this.water.autoGenerateSprings()},this.water.autoGenerateSprings(),this.setupPresetButtons(),this.setupQualitySelector(),this.setupInputModeToggle(),this.setupHomeButton(),this.setupModeSelector(),this.setupPregameModal(),this.lastTime=performance.now(),this.simAccumulator=0,this.simStep=1/60,this.timeScale=1,this.paused=!1,this.frameCount=0,this.fpsTime=0,this.currentFps=60,this.adaptiveMaxSteps=Ld(),this.targetFps=30,this.fpsHistory=[],this.fpsHistorySize=10,this.adaptiveCooldown=0,this.animate(),this.inputMode===ln.TOUCH&&this.showHelpIfFirstVisit(),console.log("Drift Unified Demo initialized"),console.log("Input mode:",this.inputMode),console.log("Quality:",gn.level,"- Grid:",this.terrain.width+"x"+this.terrain.height)}setupDesktopControlCallbacks(){this.controls.updateToolDisplay(),this.controls.onAddWater=(t,e)=>{const n=this.terrain.brushStrength*50,r=this.terrain.brushRadius;this.water.addWater(t,e,n,r)},this.controls.onAddSpring=(t,e)=>{const n=this.terrain.brushStrength*this.terrain.brushRadius*.5;this.water.addSpring(t,e,n)},this.controls.onRemoveSpring=(t,e)=>{this.water.removeSpringNear(t,e,this.terrain.brushRadius)},this.controls.onClearWater=()=>{this.water.depth.fill(0),this.water.waveEnergy.fill(0),this.water.dirty=!0},this.controls.onTimeScaleChange=t=>{this.timeScale=Math.max(.25,Math.min(20,this.timeScale+t))},this.controls.onTogglePause=()=>{this.paused=!this.paused},this.controls.onResetCamera=()=>{this.scene.resetCamera()}}setupMobileControlCallbacks(){this.controls.onAddWater=(t,e)=>{const n=this.terrain.brushStrength*50,r=this.terrain.brushRadius;this.water.addWater(t,e,n,r)},this.controls.onAddSpring=(t,e)=>{const n=this.terrain.brushStrength*this.terrain.brushRadius*.5;this.water.addSpring(t,e,n)},this.controls.onTerrainReset=()=>{this.water.reset(),this.water.autoGenerateSprings()}}setupPresetButtons(){const t=document.querySelectorAll(".preset-btn"),e=document.getElementById("preset-dropdown");t.forEach(n=>{const r=s=>{s&&(s.preventDefault(),s.stopPropagation()),t.forEach(a=>a.classList.remove("active")),n.classList.add("active");const o=n.dataset.preset;this.terrain.generate(o),this.water.reset(),this.water.autoGenerateSprings(),e&&e.classList.remove("visible")};n.addEventListener("click",r),n.addEventListener("touchend",r)})}setupQualitySelector(){const t=[document.getElementById("btn-quality"),document.getElementById("btn-quality-touch")].filter(Boolean),e=this.inputMode===ln.TOUCH?document.getElementById("quality-dropdown-touch"):document.getElementById("quality-dropdown"),n=document.querySelectorAll(".quality-btn");if(t.length===0||!e)return;const r=Ca[gn.level],s=r?r.label:"Quality";t.forEach(o=>{o.id==="btn-quality-touch"?(o.textContent="Q",o.title=s):o.textContent=s}),n.forEach(o=>{const a=o.dataset.quality;o.classList.toggle("active",a===gn.level)}),t.forEach(o=>{const a=c=>{var h,u,d;c.preventDefault(),c.stopPropagation(),(h=document.getElementById("mode-dropdown"))==null||h.classList.remove("visible"),(u=document.getElementById("mode-dropdown-touch"))==null||u.classList.remove("visible"),(d=document.getElementById("preset-dropdown"))==null||d.classList.remove("visible"),e.classList.toggle("visible")};o.addEventListener("click",a),o.addEventListener("touchend",a)}),n.forEach(o=>{const a=c=>{c.preventDefault(),c.stopPropagation();const h=o.dataset.quality;if(h!==gn.level){this.changeQuality(h);const u=Ca[h],d=u?u.label:"Quality";t.forEach(f=>{f.id==="btn-quality-touch"?f.title=d:f.textContent=d}),n.forEach(f=>{f.classList.toggle("active",f.dataset.quality===h)})}e.classList.remove("visible")};o.addEventListener("click",a),o.addEventListener("touchend",a)}),document.addEventListener("click",()=>e.classList.remove("visible")),document.addEventListener("touchend",o=>{setTimeout(()=>{const a=t.some(c=>c===o.target);!e.contains(o.target)&&!a&&e.classList.remove("visible")},50)})}setupInputModeToggle(){const t=[document.getElementById("btn-input-mode"),document.getElementById("btn-input-mode-touch")].filter(Boolean),e=pS(this.inputMode);t.forEach(n=>{n.textContent=e.icon,n.title=`Switch to ${this.inputMode===ln.DESKTOP?"Touch":"Desktop"} mode`;const r=s=>{s.preventDefault(),s.stopPropagation(),fS()};n.addEventListener("click",r),n.addEventListener("touchend",r)})}changeQuality(t){this.gameModeManager.isPlaying&&(this.gameModeManager.endMode(!1),this.showArcadeHud(!1));const e=this.terrain.getSnapshot(),n=this.water.getSnapshot();jl(t),this.terrain=new Bd,this.terrain.loadFromSnapshot(e),this.water=new kd(this.terrain),this.water.loadFromSnapshot(n),this.terrainMesh.terrain=this.terrain,this.terrainMesh.rebuild(),this.waterMesh.water=this.water,this.waterMesh.terrain=this.terrain,this.waterMesh.rebuild(),this.springMarkers.water=this.water,this.springMarkers.terrain=this.terrain,this.springMarkers.refreshScale(),this.controls.terrain=this.terrain,this.controls.terrainMesh=this.terrainMesh,this.controls.refreshScale&&this.controls.refreshScale(),this.gameModeManager.terrain=this.terrain,this.gameModeManager.water=this.water,console.log(`Quality changed to ${t}. Grid size: ${this.terrain.width}x${this.terrain.height}`)}setupHomeButton(){[document.getElementById("btn-home"),document.getElementById("btn-home-touch")].filter(Boolean).forEach(s=>{const o=a=>{a.preventDefault(),a.stopPropagation(),this.scene.resetCamera()};s.addEventListener("click",o),s.addEventListener("touchend",o)});const e=document.getElementById("close-help");if(e){const s=o=>{o.preventDefault();const a=document.getElementById("help-overlay");a&&a.classList.remove("visible")};e.addEventListener("click",s),e.addEventListener("touchend",s)}const n=document.getElementById("btn-new-map"),r=document.getElementById("preset-dropdown");n&&r&&(n.onclick=s=>{var o,a;s.preventDefault(),(o=document.getElementById("quality-dropdown-touch"))==null||o.classList.remove("visible"),(a=document.getElementById("mode-dropdown-touch"))==null||a.classList.remove("visible"),r.classList.toggle("visible")})}getModeInfo(t){return{flowRush:{icon:"🎯",title:"Flow Rush",subtitle:"2 minutes to score big",description:"Guide water to glowing targets scattered across the terrain. Build channels and redirect springs to maximize your score!",objectives:["Water touching targets earns points","Chain hits to build your combo multiplier","Use terrain tools to guide water flow","Find all targets before time runs out"]},cascade:{icon:"💧",title:"Cascade",subtitle:"Chain reactions for massive combos",description:"Water must flow through rings in sequence. Each ring you hit adds to your combo - miss the sequence and start over!",objectives:["Guide water through the highlighted ring","Rings must be hit in order (follow the sequence)","Longer chains = bigger score multipliers","Plan your terrain to create smooth water paths"]},floodDefense:{icon:"🏘️",title:"Flood Defense",subtitle:"Protect the villages from rising water",description:"Aggressive springs threaten villages in the valleys. Build barriers and redirect floods to keep settlements safe!",objectives:["Villages spawn in low ground - protect them!","Springs appear on high ground and flood downhill","Flooded villages damage your health over time","New villages and springs appear as you play"]}}[t]||null}showPregameModal(t){const e=document.getElementById("pregame-modal"),n=this.getModeInfo(t);if(!e||!n)return;e.querySelector(".mode-icon").textContent=n.icon,e.querySelector(".mode-title").textContent=n.title,e.querySelector(".mode-subtitle").textContent=n.subtitle,e.querySelector(".mode-description").textContent=n.description;const r=e.querySelector(".mode-objectives ul");r.innerHTML=n.objectives.map(s=>`<li>${s}</li>`).join(""),this.pendingGameMode=t,e.classList.add("visible")}hidePregameModal(){const t=document.getElementById("pregame-modal");t&&t.classList.remove("visible")}setupPregameModal(){const t=document.getElementById("pregame-modal");if(!t)return;const e=t.querySelector(".start-btn");if(e){const n=r=>{r.preventDefault(),this.pendingGameMode&&(this.hidePregameModal(),this.gameModeManager.startMode(this.pendingGameMode)||console.log(`Mode "${this.pendingGameMode}" not yet implemented`),this.pendingGameMode=null)};e.addEventListener("click",n),e.addEventListener("touchend",n)}}setupGameModeCallbacks(){this.gameModeManager.onModeStart=t=>{t!==ci.SANDBOX&&(this.resetGameState(),this.showArcadeHud(!0))},this.gameModeManager.onModeEnd=(t,e,n,r)=>{t!==ci.SANDBOX&&this.showGameOver(n,r)},this.gameModeManager.onScoreUpdate=(t,e)=>{this.updateArcadeHud({score:t,combo:e})}}setupModeSelector(){const t=[document.getElementById("btn-mode"),document.getElementById("btn-mode-touch")].filter(Boolean),e=this.inputMode===ln.TOUCH?document.getElementById("mode-dropdown-touch"):document.getElementById("mode-dropdown"),n=document.querySelectorAll(".mode-btn");if(t.length===0||!e)return;this.updateHighScoreDisplay(),t.forEach(o=>{const a=c=>{var h,u,d;c.preventDefault(),c.stopPropagation(),(h=document.getElementById("quality-dropdown"))==null||h.classList.remove("visible"),(u=document.getElementById("quality-dropdown-touch"))==null||u.classList.remove("visible"),(d=document.getElementById("preset-dropdown"))==null||d.classList.remove("visible"),e.classList.toggle("visible")};o.addEventListener("click",a),o.addEventListener("touchend",a)}),n.forEach(o=>{const a=c=>{c.preventDefault(),c.stopPropagation();const h=o.dataset.mode;n.forEach(d=>d.classList.remove("active")),o.classList.add("active");const u=o.querySelector(".mode-name").textContent;t.forEach(d=>d.textContent=u),e.classList.remove("visible"),h==="sandbox"?(this.gameModeManager.endMode(!1),this.showArcadeHud(!1)):this.showPregameModal(h)};o.addEventListener("click",a),o.addEventListener("touchend",a)}),document.addEventListener("click",()=>e.classList.remove("visible")),document.addEventListener("touchend",o=>{setTimeout(()=>{const a=t.some(c=>c===o.target);!e.contains(o.target)&&!a&&e.classList.remove("visible")},50)});const r=document.querySelector("#arcade-hud .restart-btn");if(r){const o=a=>{a.preventDefault(),this.restartCurrentMode()};r.addEventListener("click",o),r.addEventListener("touchend",o)}const s=document.querySelector("#arcade-hud .quit-btn");if(s){const o=a=>{a.preventDefault(),this.quitToSandbox()};s.addEventListener("click",o),s.addEventListener("touchend",o)}this.setupHighScoreEntry()}setupHighScoreEntry(){const t=document.getElementById("arcade-hud");if(!t)return;const e=t.querySelectorAll(".initials-input .initial"),n=t.querySelector(".submit-score-btn");e.forEach((s,o)=>{s.addEventListener("input",a=>{a.target.value=a.target.value.replace(/[^a-zA-Z]/g,"").toUpperCase(),a.target.value.length===1&&o<e.length-1&&e[o+1].focus()}),s.addEventListener("keydown",a=>{a.key==="Backspace"&&a.target.value===""&&o>0&&e[o-1].focus(),a.key==="Enter"&&n.click()})});const r=s=>{if(s.preventDefault(),s.stopPropagation(),n.disabled)return;n.disabled=!0;const o=Array.from(e).map(a=>a.value||"A").join("");this.submitHighScore(o),n.disabled=!1};n.addEventListener("click",r),n.addEventListener("touchend",s=>{s.preventDefault(),r(s)})}submitHighScore(t){const e=new js,n=this.gameModeManager.lastPlayedMode,r=this.pendingHighScore;n&&r>0&&(e.addHighScore(n,r,t),this.updateHighScoreDisplay(),this.showLeaderboard(n,r));const s=document.getElementById("arcade-hud");s&&(s.querySelector(".highscore-entry").classList.remove("visible"),s.querySelector(".game-buttons").style.display="flex")}showLeaderboard(t,e=null){const n=document.getElementById("arcade-hud");if(!n)return;const r=new js,s=r.getLeaderboard(t),o=n.querySelector('.leaderboard-list[data-tab-content="global"]'),a=n.querySelector('.leaderboard-list[data-tab-content="local"]'),c=n.querySelector(".leaderboard-status"),h=n.querySelectorAll(".leaderboard-tab");a.innerHTML=this.renderLeaderboardList(s,e),h.forEach(u=>{u.onclick=()=>{h.forEach(f=>f.classList.remove("active")),u.classList.add("active");const d=u.dataset.tab;o.style.display=d==="global"?"":"none",a.style.display=d==="local"?"":"none"}}),r.isGlobalLeaderboardAvailable()?(o.innerHTML='<li style="color: #666; text-align: center;">Loading...</li>',c.textContent="",r.fetchGlobalLeaderboard(t).then(u=>{u.length>0?o.innerHTML=this.renderLeaderboardList(u,e):o.innerHTML='<li style="color: #666; text-align: center;">No scores yet - be the first!</li>'}).catch(()=>{o.innerHTML='<li style="color: #666; text-align: center;">Could not load global scores</li>'}),h[0].classList.add("active"),h[1].classList.remove("active"),o.style.display="",a.style.display="none"):(o.innerHTML='<li style="color: #666; text-align: center;">Global scores unavailable</li>',c.textContent="Local scores only",h[0].classList.remove("active"),h[1].classList.add("active"),o.style.display="none",a.style.display=""),n.querySelector(".leaderboard").classList.add("visible")}renderLeaderboardList(t,e=null){return t.length===0?'<li style="color: #666; text-align: center;">No scores yet</li>':t.map((n,r)=>`
        <li class="${n.score===e?"highlight":""}">
          <span class="rank">${r+1}.</span>
          <span class="initials">${n.initials}</span>
          <span class="score">${n.score.toLocaleString()}</span>
        </li>
      `).join("")}updateHighScoreDisplay(){const t=new js;["flowRush","cascade","floodDefense"].forEach(n=>{const r=t.getHighScore(n),s=r>0?`Best: ${r}`:"",o=document.getElementById(`highscore-${n}`);o&&(o.textContent=s);const a=document.getElementById(`highscore-${n}-touch`);a&&(a.textContent=s)})}showArcadeHud(t){const e=document.getElementById("arcade-hud");e&&(e.classList.toggle("visible",t),e.classList.remove("game-ended"),t&&(e.querySelector(".game-over").style.display="none",e.querySelector(".final-score").style.display="none",e.querySelector(".new-high-score").style.display="none",e.querySelector(".game-buttons").style.display="none",e.querySelector(".highscore-entry").classList.remove("visible"),e.querySelector(".leaderboard").classList.remove("visible"),e.querySelector(".score").textContent="0",e.querySelector(".combo").textContent="x1",e.querySelector(".combo-bar-fill").style.width="0%",e.querySelector(".timer").textContent="2:00",e.querySelector(".timer").classList.remove("urgent"),e.querySelector(".lives").innerHTML="",this.pendingHighScore=null))}updateArcadeHud(t){const e=document.getElementById("arcade-hud");if(e){if(t.score!==void 0&&(e.querySelector(".score").textContent=t.score),t.combo!==void 0&&(e.querySelector(".combo").textContent=`x${t.combo}`),t.comboTimer!==void 0&&(e.querySelector(".combo-bar-fill").style.width=`${t.comboTimer*100}%`),t.quarterHearts!==void 0){const n=e.querySelector(".lives");n.innerHTML=this.renderQuarterHearts(t.quarterHearts,t.maxQuarterHearts)}else if(t.lives!==void 0){const n=e.querySelector(".lives");n.textContent=Array(t.lives).fill("❤").join(" ")}if(t.timeRemaining!==void 0){const n=e.querySelector(".timer"),r=Math.floor(t.timeRemaining/60),s=Math.floor(t.timeRemaining%60);n.textContent=`${r}:${s.toString().padStart(2,"0")}`,n.classList.toggle("urgent",t.timeRemaining<30)}}}showGameOver(t,e){const n=document.getElementById("arcade-hud");if(!n)return;n.classList.add("game-ended");const r=n.querySelector(".game-over");this.gameModeManager.currentMode&&this.gameModeManager.currentMode.victory?(r.textContent="TIME'S UP!",r.style.color="#88ccff"):(r.textContent="GAME OVER",r.style.color="#ff6666"),r.style.display="block",n.querySelector(".final-score").style.display="block",n.querySelector(".final-score").textContent=`Final Score: ${t}`;const s=new js,o=this.gameModeManager.lastPlayedMode||this.gameModeManager.currentModeType;if(s.isTopScore(o,t)){n.querySelector(".new-high-score").style.display="block",n.querySelector(".highscore-entry").classList.add("visible"),n.querySelector(".game-buttons").style.display="none",this.pendingHighScore=t;const c=n.querySelectorAll(".initials-input .initial");c.forEach(u=>u.value=""),c[0].focus();const h=s.getRankForScore(o,t);n.querySelector(".rank-text").textContent=`#${h} - Enter your initials:`}else n.querySelector(".game-buttons").style.display="flex",s.getLeaderboard(o).length>0&&this.showLeaderboard(o)}restartCurrentMode(){const t=this.gameModeManager.lastPlayedMode;t&&(this.hideGameOverUI(),this.showPregameModal(t))}hideGameOverUI(){const t=document.getElementById("arcade-hud");t&&(t.classList.remove("game-ended"),t.querySelector(".game-over").style.display="none",t.querySelector(".final-score").style.display="none",t.querySelector(".new-high-score").style.display="none",t.querySelector(".highscore-entry").classList.remove("visible"),t.querySelector(".leaderboard").classList.remove("visible"),t.querySelector(".game-buttons").style.display="none",t.classList.remove("visible"))}quitToSandbox(){this.showArcadeHud(!1),this.gameModeManager.endMode(!1),[document.getElementById("btn-mode"),document.getElementById("btn-mode-touch")].filter(Boolean).forEach(n=>n.textContent="Sandbox"),document.querySelectorAll(".mode-btn").forEach(n=>{n.classList.toggle("active",n.dataset.mode==="sandbox")})}resetGameState(){this.timeScale=1,this.paused=!1,this.terrain.brushRadius=8,this.terrain.brushStrength=2,this.inputMode===ln.DESKTOP?(this.controls.currentTool="raise",this.controls.updateToolDisplay()):(this.controls.currentTool="raise",document.querySelectorAll(".tool-btn").forEach(e=>{e.classList.toggle("active",e.dataset.tool==="raise")}))}renderQuarterHearts(t,e){const n=Math.ceil(e/4);let r="";for(let s=0;s<n;s++){const o=Math.max(0,Math.min(4,t-s*4));o===4?r+='<span class="heart full">❤</span>':o===3?r+='<span class="heart three-quarter">❤</span>':o===2?r+='<span class="heart half">❤</span>':o===1?r+='<span class="heart quarter">❤</span>':r+='<span class="heart empty">♡</span>'}return r}showHelpIfFirstVisit(){if(!localStorage.getItem("drift-unified-visited")){const e=document.getElementById("help-overlay");e&&e.classList.add("visible"),localStorage.setItem("drift-unified-visited","true")}}animate(){requestAnimationFrame(()=>this.animate());const t=performance.now(),e=(t-this.lastTime)/1e3;this.lastTime=t;const n=this.paused?0:e*this.timeScale;if(this.frameCount++,this.fpsTime+=e,this.fpsTime>=.5){if(this.currentFps=this.frameCount/this.fpsTime,this.frameCount=0,this.fpsTime=0,this.fpsHistory.push(this.currentFps),this.fpsHistory.length>this.fpsHistorySize&&this.fpsHistory.shift(),this.adaptiveCooldown>0)this.adaptiveCooldown--;else if(this.fpsHistory.length>=3){const s=this.fpsHistory.reduce((c,h)=>c+h,0)/this.fpsHistory.length,o=Math.min(...this.fpsHistory.slice(-3)),a=Ld();o<this.targetFps&&this.adaptiveMaxSteps>1?(this.adaptiveMaxSteps=Math.max(1,this.adaptiveMaxSteps-1),this.adaptiveCooldown=4,console.log(`Adaptive: FPS ${o.toFixed(0)} < ${this.targetFps}, reducing steps to ${this.adaptiveMaxSteps}`)):s>this.targetFps+15&&this.adaptiveMaxSteps<a&&(this.adaptiveMaxSteps=Math.min(a,this.adaptiveMaxSteps+1),this.adaptiveCooldown=6,console.log(`Adaptive: FPS ${s.toFixed(0)} good, increasing steps to ${this.adaptiveMaxSteps}`))}this.controls.updateStats&&(this.inputMode===ln.DESKTOP?this.controls.updateStats(this.currentFps,this.water.getTotalWater(),this.water.springs.length,this.timeScale,this.paused):this.controls.updateStats(this.currentFps,this.water.getTotalWater(),this.water.springs.length))}this.simAccumulator+=n;let r=0;for(;this.simAccumulator>=this.simStep&&r<this.adaptiveMaxSteps;)this.water.simulate(this.simStep),this.simAccumulator-=this.simStep,r++;if(this.simAccumulator>this.simStep*4&&(this.simAccumulator=this.simStep*2),this.gameModeManager.update(n),this.gameModeManager.isPlaying&&this.gameModeManager.currentMode){const s=this.gameModeManager.currentMode.getDisplayInfo();this.updateArcadeHud(s)}this.inputMode===ln.DESKTOP&&this.controls.updateCamera&&this.controls.updateCamera(e),this.terrainMesh.update(),this.waterMesh.update(n),this.springMarkers.update(n),this.scene.update(n),this.scene.render()}}document.readyState==="loading"?document.addEventListener("DOMContentLoaded",()=>new Gf):new Gf;
