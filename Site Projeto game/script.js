// ===== Digitação no título principal =====
function digitarTitulo(texto, elemento, velocidade = 100) {
    let i = 0;
    const escrever = () => {
        if (i < texto.length) {
            elemento.textContent += texto.charAt(i);
            i++;
            setTimeout(escrever, velocidade);
        }
    };
    elemento.textContent = "";
    escrever();
}
const titulo = document.querySelector("header h1");
digitarTitulo("Prismatic Games", titulo, 100);

// ===== Botão de volta ao topo (criado dinamicamente) =====
const botaoTopo = document.createElement("button");
botaoTopo.textContent = "⬆️ Topo";
botaoTopo.style.position = "fixed";
botaoTopo.style.bottom = "20px";
botaoTopo.style.right = "20px";
botaoTopo.style.padding = "10px 15px";
botaoTopo.style.backgroundColor = "#3a8dd8";
botaoTopo.style.color = "#fff";
botaoTopo.style.border = "none";
botaoTopo.style.borderRadius = "8px";
botaoTopo.style.cursor = "pointer";
botaoTopo.style.display = "none";
botaoTopo.style.zIndex = "1000";
botaoTopo.style.boxShadow = "0 4px 8px rgba(0,0,0,0.2)";
document.body.appendChild(botaoTopo);

window.addEventListener("scroll", () => {
    botaoTopo.style.display = window.scrollY > 300 ? "block" : "none";
});
botaoTopo.addEventListener("click", () => {
    window.scrollTo({ top: 0, behavior: "smooth" });
});

// ===== Animação de rotação nos personagens ao passar o mouse =====
const personagens = document.querySelectorAll(".personagem");
personagens.forEach(personagem => {
    personagem.addEventListener("mouseenter", () => {
        personagem.style.transition = "transform 0.5s";
        personagem.style.transform = "rotate(1deg) scale(1.02)";
    });
    personagem.addEventListener("mouseleave", () => {
        personagem.style.transform = "rotate(0deg) scale(1)";
    });
});

// ===== Muda cor de fundo com base na seção visível =====
const secoes = document.querySelectorAll("section");
window.addEventListener("scroll", () => {
    secoes.forEach(secao => {
        const top = window.scrollY;
        const offset = secao.offsetTop - 100;
        const height = secao.offsetHeight;
        if (top >= offset && top < offset + height) {
            document.body.style.backgroundColor = getComputedStyle(secao).backgroundColor;
        }
    });
});

// ===== Animação nos links do menu ao clicar =====
const navLinks = document.querySelectorAll("nav a");
navLinks.forEach(link => {
    link.addEventListener("click", () => {
        link.style.transform = "scale(0.95)";
        setTimeout(() => {
            link.style.transform = "scale(1)";
        }, 150);
    });
});

// ===== Troca o título da aba quando o usuário sai da página =====
const tituloOriginal = document.title;
document.addEventListener("visibilitychange", () => {
    document.title = document.hidden ? "👀 Volte pro jogo!" : tituloOriginal;
});

// ===== Animação suave de fade-in quando carrega os personagens =====
window.addEventListener("load", () => {
    personagens.forEach((el, i) => {
        el.style.opacity = "0";
        el.style.transition = "opacity 0.6s ease";
        setTimeout(() => {
            el.style.opacity = "1";
        }, 150 * i);
    });
});
