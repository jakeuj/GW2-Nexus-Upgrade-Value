(() => {
  "use strict";

  const root = document.documentElement;
  const reduceMotion = window.matchMedia("(prefers-reduced-motion: reduce)");
  const finePointer = window.matchMedia("(pointer: fine)");

  root.classList.add("js");

  const setRevealDelays = () => {
    document.querySelectorAll("[data-reveal-delay]").forEach((element) => {
      const delay = Number.parseInt(element.dataset.revealDelay || "0", 10);
      if (Number.isFinite(delay)) {
        element.style.setProperty("--reveal-delay", `${Math.max(0, delay)}ms`);
      }
    });
  };

  const revealContent = () => {
    const targets = document.querySelectorAll(".reveal");
    if (reduceMotion.matches || !("IntersectionObserver" in window)) {
      targets.forEach((target) => target.classList.add("is-visible"));
      return;
    }

    const observer = new IntersectionObserver((entries) => {
      entries.forEach((entry) => {
        if (!entry.isIntersecting) return;
        entry.target.classList.add("is-visible");
        observer.unobserve(entry.target);
      });
    }, { rootMargin: "0px 0px -9%", threshold: 0.08 });

    targets.forEach((target) => observer.observe(target));
  };

  const animateCounter = (element) => {
    if (element.dataset.animated === "true") return;

    const target = Number.parseInt(element.dataset.count || "0", 10);
    const suffix = element.dataset.suffix || "";
    if (!Number.isFinite(target)) return;

    element.dataset.animated = "true";
    if (reduceMotion.matches) {
      element.textContent = `${target}${suffix}`;
      return;
    }

    const duration = 850;
    let startedAt = 0;
    const step = (timestamp) => {
      if (!startedAt) startedAt = timestamp;
      const progress = Math.min((timestamp - startedAt) / duration, 1);
      const eased = 1 - Math.pow(1 - progress, 3);
      element.textContent = `${Math.round(target * eased)}${suffix}`;
      if (progress < 1) window.requestAnimationFrame(step);
    };
    window.requestAnimationFrame(step);
  };

  const initializeCounters = () => {
    const counters = document.querySelectorAll("[data-count]");
    if (reduceMotion.matches || !("IntersectionObserver" in window)) {
      counters.forEach(animateCounter);
      return;
    }

    const observer = new IntersectionObserver((entries) => {
      entries.forEach((entry) => {
        if (!entry.isIntersecting) return;
        animateCounter(entry.target);
        observer.unobserve(entry.target);
      });
    }, { threshold: 0.7 });

    counters.forEach((counter) => observer.observe(counter));
  };

  const initializeFlow = () => {
    const flows = document.querySelectorAll("[data-flow]");
    if (reduceMotion.matches || !("IntersectionObserver" in window)) return;

    const observer = new IntersectionObserver((entries) => {
      entries.forEach((entry) => {
        if (!entry.isIntersecting) return;
        entry.target.classList.add("is-flowing");
        observer.unobserve(entry.target);
      });
    }, { rootMargin: "0px 0px -14%", threshold: 0.32 });

    flows.forEach((flow) => observer.observe(flow));
  };

  const initializeNavigation = () => {
    const button = document.querySelector(".nav-toggle");
    const nav = document.querySelector("#primary-nav");
    if (!button || !nav) return;

    const closeMenu = (restoreFocus = false) => {
      button.setAttribute("aria-expanded", "false");
      button.querySelector(".sr-only").textContent = "開啟導覽選單";
      nav.classList.remove("is-open");
      document.body.classList.remove("nav-open");
      if (restoreFocus) button.focus();
    };

    button.addEventListener("click", () => {
      const isOpen = button.getAttribute("aria-expanded") === "true";
      button.setAttribute("aria-expanded", String(!isOpen));
      button.querySelector(".sr-only").textContent = isOpen ? "開啟導覽選單" : "關閉導覽選單";
      nav.classList.toggle("is-open", !isOpen);
      document.body.classList.toggle("nav-open", !isOpen);
    });

    nav.querySelectorAll("a").forEach((link) => {
      link.addEventListener("click", () => closeMenu());
    });

    document.addEventListener("keydown", (event) => {
      if (event.key === "Escape" && button.getAttribute("aria-expanded") === "true") {
        closeMenu(true);
      }
    });

    const sectionLinks = [...nav.querySelectorAll("a[href^='#']")];
    const sections = sectionLinks
      .map((link) => document.querySelector(link.getAttribute("href")))
      .filter(Boolean);

    if (!("IntersectionObserver" in window)) return;

    const spy = new IntersectionObserver((entries) => {
      const visible = entries
        .filter((entry) => entry.isIntersecting)
        .sort((a, b) => b.intersectionRatio - a.intersectionRatio)[0];
      if (!visible) return;

      sectionLinks.forEach((link) => {
        const active = link.getAttribute("href") === `#${visible.target.id}`;
        if (active) link.setAttribute("aria-current", "true");
        else link.removeAttribute("aria-current");
      });
    }, { rootMargin: "-20% 0px -65%", threshold: [0.05, 0.2, 0.5] });

    sections.forEach((section) => spy.observe(section));
  };

  const initializeScrollProgress = () => {
    let ticking = false;

    const update = () => {
      const max = Math.max(1, document.documentElement.scrollHeight - window.innerHeight);
      root.style.setProperty("--scroll-progress", String(Math.min(1, window.scrollY / max)));
      ticking = false;
    };

    window.addEventListener("scroll", () => {
      if (ticking) return;
      ticking = true;
      window.requestAnimationFrame(update);
    }, { passive: true });

    window.addEventListener("resize", update, { passive: true });
    update();
  };

  const initializePointerEffects = () => {
    if (!finePointer.matches || reduceMotion.matches) return;

    document.querySelectorAll("[data-tilt]").forEach((card) => {
      card.addEventListener("pointermove", (event) => {
        const rect = card.getBoundingClientRect();
        const x = (event.clientX - rect.left) / rect.width - 0.5;
        const y = (event.clientY - rect.top) / rect.height - 0.5;
        card.style.setProperty("--tilt-y", `${(x * 4).toFixed(2)}deg`);
        card.style.setProperty("--tilt-x", `${(-y * 3).toFixed(2)}deg`);
      });
      card.addEventListener("pointerleave", () => {
        card.style.removeProperty("--tilt-x");
        card.style.removeProperty("--tilt-y");
      });
    });

    document.querySelectorAll(".spotlight").forEach((card) => {
      card.addEventListener("pointermove", (event) => {
        const rect = card.getBoundingClientRect();
        card.style.setProperty("--pointer-x", `${event.clientX - rect.left}px`);
        card.style.setProperty("--pointer-y", `${event.clientY - rect.top}px`);
      });
    });

    const hero = document.querySelector(".hero");
    if (hero) {
      hero.addEventListener("pointermove", (event) => {
        const x = event.clientX / window.innerWidth - 0.5;
        const y = event.clientY / window.innerHeight - 0.5;
        hero.style.setProperty("--art-x", `${(x * -10).toFixed(1)}px`);
        hero.style.setProperty("--art-y", `${(y * -7).toFixed(1)}px`);
      });
      hero.addEventListener("pointerleave", () => {
        hero.style.removeProperty("--art-x");
        hero.style.removeProperty("--art-y");
      });
    }
  };

  setRevealDelays();
  revealContent();
  initializeCounters();
  initializeFlow();
  initializeNavigation();
  initializeScrollProgress();
  initializePointerEffects();
})();
