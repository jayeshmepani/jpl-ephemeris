'use strict';

function initTheme() {
    const stored = localStorage.getItem('theme');
    const prefersDark = window.matchMedia('(prefers-color-scheme: dark)').matches;
    applyTheme(stored || (prefersDark ? 'dark' : 'light'), false);

    document.querySelectorAll('#themeToggle, #mobileThemeToggle').forEach((toggle) => {
        toggle.addEventListener('click', () => {
            const current = document.documentElement.getAttribute('data-theme');
            applyTheme(current === 'dark' ? 'light' : 'dark');
        });
    });

    window.matchMedia('(prefers-color-scheme: dark)').addEventListener('change', (event) => {
        if (!localStorage.getItem('theme')) {
            applyTheme(event.matches ? 'dark' : 'light', false);
        }
    });
}

function applyTheme(theme, save = true) {
    document.documentElement.setAttribute('data-theme', theme);
    if (save) localStorage.setItem('theme', theme);

    const isDark = theme === 'dark';
    const label = isDark ? 'Switch to light theme' : 'Switch to dark theme';
    const meta = document.querySelector('meta[name="theme-color"]');
    if (meta) meta.setAttribute('content', isDark ? '#141816' : '#f6f3ec');

    document.querySelectorAll('#themeToggle, #mobileThemeToggle').forEach((toggle) => {
        toggle.setAttribute('aria-label', label);
        toggle.setAttribute('aria-pressed', String(isDark));
        const badge = toggle.querySelector('.theme-badge');
        if (badge) badge.textContent = isDark ? 'Theme: Dark' : 'Theme: Light';
    });
}

function setCurrentLink(links, href) {
    links.forEach((link) => {
        const on = link.getAttribute('href') === href;
        link.classList.toggle('active', on);
        if (on) link.setAttribute('aria-current', 'location');
        else link.removeAttribute('aria-current');
    });
}

function initNavigation() {
    const sidebar = document.getElementById('sidebar');
    const mobileToggle = document.getElementById('mobileSidebarToggle');
    const overlay = document.getElementById('sidebarOverlay');
    const main = document.getElementById('main-content');
    const navLinks = [...document.querySelectorAll('.nav-link')];
    const sectionLinks = navLinks.filter((link) => {
        const id = link.getAttribute('href')?.slice(1);
        const target = id ? document.getElementById(id) : null;
        return target?.classList.contains('content-section');
    });
    const isMobileQuery = window.matchMedia('(max-width: 768px)');

    function closeSidebar() {
        const returnFocus = isMobileQuery.matches && sidebar.classList.contains('open');
        sidebar.classList.remove('open');
        overlay?.classList.remove('active');
        mobileToggle?.setAttribute('aria-expanded', 'false');
        document.body.style.overflow = '';
        main?.removeAttribute('inert');
        if (returnFocus) mobileToggle?.focus();
        if (isMobileQuery.matches) sidebar.setAttribute('inert', '');
    }

    function openSidebar() {
        sidebar.classList.add('open');
        sidebar.removeAttribute('inert');
        overlay?.classList.add('active');
        mobileToggle?.setAttribute('aria-expanded', 'true');
        document.body.style.overflow = 'hidden';
        if (isMobileQuery.matches) main?.setAttribute('inert', '');
        requestAnimationFrame(() => {
            sidebar.querySelector('#sidebarSearch, .nav-link')?.focus();
        });
    }

    function handleMobileState() {
        if (!isMobileQuery.matches || sidebar.classList.contains('open')) {
            if (!isMobileQuery.matches) {
                sidebar.classList.remove('open');
                overlay?.classList.remove('active');
                sidebar.removeAttribute('inert');
                main?.removeAttribute('inert');
                document.body.style.overflow = '';
                mobileToggle?.setAttribute('aria-expanded', 'false');
            }
            return;
        }
        sidebar.setAttribute('inert', '');
    }

    handleMobileState();
    isMobileQuery.addEventListener('change', handleMobileState);

    mobileToggle?.addEventListener('click', () => {
        if (sidebar.classList.contains('open')) closeSidebar();
        else openSidebar();
    });

    document.getElementById('sidebarClose')?.addEventListener('click', closeSidebar);

    overlay?.addEventListener('click', closeSidebar);

    document.addEventListener('keydown', (event) => {
        if (event.key === 'Escape' && sidebar.classList.contains('open')) closeSidebar();
    });

    navLinks.forEach((link) => {
        link.addEventListener('click', () => {
            if (isMobileQuery.matches) closeSidebar();
        });
    });

    const hash = window.location.hash;
    const hashed = hash ? navLinks.find((link) => link.getAttribute('href') === hash) : null;
    if (hashed) setCurrentLink(navLinks, hash);
    else setCurrentLink(sectionLinks, '#introduction');

    const sections = [...document.querySelectorAll('.content-section')];
    const observer = new IntersectionObserver(
        (entries) => {
            const visible = entries.filter((entry) => entry.isIntersecting).sort((a, b) => a.boundingClientRect.top - b.boundingClientRect.top);
            if (!visible.length) return;
            setCurrentLink(sectionLinks, `#${visible[0].target.id}`);
        },
        { rootMargin: '-15% 0px -75% 0px', threshold: 0 }
    );

    sections.forEach((section) => observer.observe(section));
}

const COPY_ICON =
    '<svg xmlns="http://www.w3.org/2000/svg" width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true"><rect x="9" y="9" width="13" height="13" rx="2" ry="2"/><path d="M5 15H4a2 2 0 0 1-2-2V4a2 2 0 0 1 2-2h9a2 2 0 0 1 2 2v1"/></svg>';
const CHECK_ICON =
    '<svg xmlns="http://www.w3.org/2000/svg" width="13" height="13" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true"><polyline points="20 6 9 17 4 12"/></svg>';

function initCodeBlocks() {
    document.querySelectorAll('pre').forEach((pre) => {
        pre.removeAttribute('style');
        pre.querySelectorAll('[style]').forEach((el) => el.removeAttribute('style'));

        const wrapper = pre.closest('.code-block-wrapper');
        if (wrapper) {
            const existing = wrapper.querySelector('.copy-btn');
            if (existing && !existing.dataset.wired) wireCopyBtn(existing, pre);
            return;
        }

        const newWrapper = document.createElement('div');
        newWrapper.className = 'code-block-wrapper';
        pre.parentNode.insertBefore(newWrapper, pre);
        newWrapper.appendChild(pre);

        const lang = (pre.querySelector('code')?.className || '').match(/language-(\w+)/)?.[1]?.toUpperCase() || 'CODE';

        const header = document.createElement('div');
        header.className = 'code-header';

        const langSpan = document.createElement('span');
        langSpan.className = 'code-lang';
        langSpan.textContent = lang;

        const copyBtn = document.createElement('button');
        copyBtn.className = 'copy-btn';
        copyBtn.type = 'button';
        copyBtn.setAttribute('aria-label', 'Copy code');
        copyBtn.innerHTML = `${COPY_ICON} Copy`;

        header.append(langSpan, copyBtn);
        newWrapper.insertBefore(header, pre);
        wireCopyBtn(copyBtn, pre);
    });
}

function wireCopyBtn(btn, pre) {
    btn.dataset.wired = 'true';
    btn.addEventListener('click', async () => {
        const code = pre.querySelector('code') || pre;
        const text = code.textContent || '';

        try {
            await navigator.clipboard.writeText(text);
        } catch {
            const range = document.createRange();
            range.selectNodeContents(code);
            const selection = window.getSelection();
            selection.removeAllRanges();
            selection.addRange(range);
            document.execCommand('copy');
            selection.removeAllRanges();
        }

        btn.innerHTML = `${CHECK_ICON} Copied`;
        btn.classList.add('copied');
        btn.setAttribute('aria-label', 'Copied');

        clearTimeout(btn.copyTimer);
        btn.copyTimer = setTimeout(() => {
            btn.innerHTML = `${COPY_ICON} Copy`;
            btn.classList.remove('copied');
            btn.setAttribute('aria-label', 'Copy code');
        }, 2200);
    });
}

function initSidebarSearch() {
    const search = document.getElementById('sidebarSearch');
    const status = document.getElementById('docSearchStatus');
    const results = document.getElementById('docSearchResults');
    const root = document.getElementById('main-content');
    if (!search || !root) return;

    const highlightApi = Boolean(window.CSS?.highlights && typeof Highlight === 'function');
    let groups = [];
    let current = -1;
    let currentMark = null;

    function clearHighlights() {
        document.querySelectorAll('.is-search-current').forEach((node) => node.classList.remove('is-search-current'));
        if (highlightApi) {
            CSS.highlights.delete('jme-find');
            CSS.highlights.delete('jme-find-current');
        }
        root.querySelectorAll('mark.find-hit').forEach((mark) => {
            mark.replaceWith(document.createTextNode(mark.textContent));
        });
        root.normalize();
        currentMark = null;
    }

    function textNodes() {
        const nodes = [];
        const walker = document.createTreeWalker(root, NodeFilter.SHOW_TEXT, {
            acceptNode(node) {
                if (!node.nodeValue || !node.nodeValue.trim()) return NodeFilter.FILTER_REJECT;
                const parent = node.parentElement;
                if (!parent || parent.closest('script, style, .doc-search-results')) return NodeFilter.FILTER_REJECT;
                return NodeFilter.FILTER_ACCEPT;
            }
        });
        while (walker.nextNode()) nodes.push(walker.currentNode);
        return nodes;
    }

    function blockFor(node) {
        const element = node.parentElement;
        return element?.closest('article.method, tr, .shared-call, .callout, .doc-card, .ref-group-head') || element?.closest('p, li, h2, h3') || element;
    }

    function labelFor(block) {
        const heading = block.matches?.('h2, h3, h4') ? block : block.querySelector?.('h2, h3, h4') || block.closest('section')?.querySelector('h2, h3');
        return (heading?.textContent || 'On this page').replace(/\s+/g, ' ').trim();
    }

    function reveal(node) {
        const panel = node.parentElement?.closest('.tab-panel');
        if (panel && !panel.classList.contains('active')) {
            panel.closest('[data-tabs]')?.querySelector(`[aria-controls="${panel.id}"]`)?.click();
        }
        let element = node.parentElement;
        while (element && element !== document.body) {
            if (element.hidden) element.hidden = false;
            element = element.parentElement;
        }
    }

    function snippet(text, index, length) {
        const start = Math.max(0, index - 32);
        const end = Math.min(text.length, index + length + 40);
        let value = text.slice(start, end).replace(/\s+/g, ' ').trim();
        if (start > 0) value = `…${value}`;
        if (end < text.length) value = `${value}…`;
        return value;
    }

    function focusSearch() {
        const mobile = window.matchMedia('(max-width: 768px)').matches;
        const sidebar = document.getElementById('sidebar');
        if (mobile && sidebar && !sidebar.classList.contains('open')) {
            document.getElementById('mobileSidebarToggle')?.click();
        }
        search.focus();
    }

    function showCurrent(scroll) {
        const group = groups[current];
        if (!group) return;
        if (scroll) reveal(group.node);
        document.querySelectorAll('.is-search-current').forEach((node) => node.classList.remove('is-search-current'));
        group.block?.classList.add('is-search-current');
        results.querySelectorAll('.doc-search-hit').forEach((button) => {
            const on = Number(button.dataset.index) === current;
            if (on) button.setAttribute('aria-current', 'true');
            else button.removeAttribute('aria-current');
        });
        if (highlightApi) {
            CSS.highlights.delete('jme-find-current');
            if (group.ranges[0]) {
                const currentHighlight = new Highlight(group.ranges[0]);
                currentHighlight.priority = 2;
                CSS.highlights.set('jme-find-current', currentHighlight);
            }
        } else if (group.marks?.[0]) {
            if (currentMark) currentMark.classList.remove('current');
            currentMark = group.marks[0];
            currentMark.classList.add('current');
        }
        if (scroll && group.block) {
            const scrollingRoot = document.documentElement;
            const previous = scrollingRoot.style.scrollBehavior;
            scrollingRoot.style.scrollBehavior = 'auto';
            group.block.scrollIntoView({ block: 'start', behavior: 'instant' });
            scrollingRoot.style.scrollBehavior = previous;
        }
    }

    function renderResults() {
        results.replaceChildren();
        const listed = groups.slice(0, 40);
        listed.forEach((group, index) => {
            const item = document.createElement('li');
            const button = document.createElement('button');
            button.type = 'button';
            button.className = 'doc-search-hit';
            button.dataset.index = String(index);
            const title = document.createElement('span');
            title.textContent = group.label;
            const detail = document.createElement('small');
            detail.textContent = group.snippet;
            button.append(title, detail);
            button.addEventListener('click', () => {
                current = index;
                showCurrent(true);
            });
            item.append(button);
            results.append(item);
        });
    }

    function move(step) {
        if (!groups.length) return;
        current = (current + step + groups.length) % groups.length;
        showCurrent(true);
    }

    function paint(query) {
        clearHighlights();
        groups = [];
        current = -1;
        if (query.length < 2) {
            if (status) status.textContent = query.length ? 'Type at least 2 characters.' : '';
            results.replaceChildren();
            return;
        }

        const needle = query.toLowerCase();
        const ranges = [];
        const marks = [];
        const grouped = new Map();
        let matchCount = 0;
        const cap = 400;

        textNodes().forEach((node) => {
            const text = node.nodeValue;
            const lower = text.toLowerCase();
            let from = 0;
            while (matchCount < cap) {
                const index = lower.indexOf(needle, from);
                if (index < 0) break;
                matchCount += 1;
                const block = blockFor(node);
                let group = grouped.get(block);
                if (!group) {
                    group = {
                        block,
                        node,
                        label: labelFor(block),
                        snippet: snippet(text, index, needle.length),
                        ranges: [],
                        marks: []
                    };
                    grouped.set(block, group);
                    groups.push(group);
                }
                if (highlightApi) {
                    const range = new Range();
                    range.setStart(node, index);
                    range.setEnd(node, index + needle.length);
                    group.ranges.push(range);
                    ranges.push(range);
                } else {
                    marks.push({ node, index, length: needle.length, group });
                }
                from = index + needle.length;
            }
        });

        if (!highlightApi) {
            const byNode = new Map();
            marks.forEach((mark) => {
                const list = byNode.get(mark.node) || [];
                list.push(mark);
                byNode.set(mark.node, list);
            });
            byNode.forEach((list) => {
                const node = list[0].node;
                const text = node.nodeValue;
                const fragment = document.createDocumentFragment();
                let cursor = 0;
                list.sort((a, b) => a.index - b.index).forEach((mark) => {
                    if (mark.index < cursor) return;
                    fragment.append(document.createTextNode(text.slice(cursor, mark.index)));
                    const hit = document.createElement('mark');
                    hit.className = 'find-hit';
                    hit.textContent = text.slice(mark.index, mark.index + mark.length);
                    mark.group.marks.push(hit);
                    fragment.append(hit);
                    cursor = mark.index + mark.length;
                });
                fragment.append(document.createTextNode(text.slice(cursor)));
                node.parentNode.replaceChild(fragment, node);
            });
        } else if (ranges.length) {
            const all = new Highlight(...ranges);
            all.priority = 1;
            CSS.highlights.set('jme-find', all);
        }

        const extra = matchCount >= cap ? ` Showing the first ${cap}.` : '';
        if (status) {
            status.textContent = groups.length
                ? `${matchCount} ${matchCount === 1 ? 'match' : 'matches'} in ${groups.length} ${groups.length === 1 ? 'place' : 'places'}.${extra}`
                : 'No matches on this page.';
        }
        renderResults();
        if (groups.length) {
            current = 0;
            showCurrent(false);
        }
    }

    const run = debounce(() => paint(search.value.trim()), 140);
    search.addEventListener('input', run);
    search.addEventListener('keydown', (event) => {
        if (event.key === 'Escape' && search.value) {
            event.stopPropagation();
            search.value = '';
            run.cancel();
            paint('');
            return;
        }
        if (event.key === 'Enter') {
            event.preventDefault();
            move(event.shiftKey ? -1 : 1);
        }
    });
    document.getElementById('mobileSearchOpen')?.addEventListener('click', focusSearch);
    document.addEventListener('keydown', (event) => {
        if (event.key !== '/' || event.metaKey || event.ctrlKey || event.altKey) return;
        const active = document.activeElement;
        const tag = active?.tagName;
        if (tag === 'INPUT' || tag === 'TEXTAREA' || active?.isContentEditable) return;
        event.preventDefault();
        focusSearch();
    });
}

function handleResponsiveTables() {
    document.querySelectorAll('.data-table').forEach((table) => {
        if (table.closest('.table-scroll')) return;
        const wrapper = document.createElement('div');
        wrapper.className = 'table-scroll';
        const caption = table.querySelector('caption')?.textContent.trim();
        table.parentNode.insertBefore(wrapper, table);
        wrapper.appendChild(table);

        const mark = () => {
            const overflows = wrapper.scrollWidth > wrapper.clientWidth + 1;
            if (overflows) {
                wrapper.tabIndex = 0;
                wrapper.setAttribute('role', 'region');
                wrapper.setAttribute('aria-label', caption || 'Scrollable table');
            } else {
                wrapper.removeAttribute('tabindex');
                wrapper.removeAttribute('role');
                wrapper.removeAttribute('aria-label');
            }
        };
        mark();
        window.addEventListener('resize', mark, { passive: true });
    });
}

function initInstallTabs() {
    document.querySelectorAll('[data-tabs]').forEach((group) => {
        const buttons = [...group.querySelectorAll('.tab-button')];
        const panels = [...group.querySelectorAll('.tab-panel')];
        if (!buttons.length) return;

        function select(button, moveFocus) {
            const targetId = button.getAttribute('aria-controls');
            buttons.forEach((item) => {
                const active = item === button;
                item.classList.toggle('active', active);
                item.setAttribute('aria-selected', String(active));
                item.tabIndex = active ? 0 : -1;
            });
            panels.forEach((panel) => {
                const active = panel.id === targetId;
                panel.classList.toggle('active', active);
            });
            if (moveFocus) button.focus();
        }

        buttons.forEach((button, index) => {
            button.addEventListener('click', () => select(button, false));
            button.addEventListener('keydown', (event) => {
                const keys = ['ArrowLeft', 'ArrowRight', 'Home', 'End'];
                if (!keys.includes(event.key)) return;
                event.preventDefault();
                let next = index;
                if (event.key === 'ArrowRight') next = (index + 1) % buttons.length;
                if (event.key === 'ArrowLeft') next = (index - 1 + buttons.length) % buttons.length;
                if (event.key === 'Home') next = 0;
                if (event.key === 'End') next = buttons.length - 1;
                select(buttons[next], true);
            });
        });

        group.classList.add('tabs-ready');
        const current = buttons.find((button) => button.classList.contains('active')) || buttons[0];
        select(current, false);
    });
}

function initPageToc() {
    const tocNav = document.getElementById('pageTocNav');
    const label = document.querySelector('.page-toc-label');
    if (!tocNav || !label) return;

    const toc = document.querySelector('.page-toc');
    const sections = [...document.querySelectorAll('.content-section[id]')];
    let headingObserver;

    function render(section) {
        const headings = [...section.querySelectorAll('h3[id]')];
        tocNav.replaceChildren();
        if (headingObserver) headingObserver.disconnect();
        if (!headings.length) {
            if (toc) toc.hidden = true;
            return;
        }

        if (toc) toc.hidden = false;
        label.textContent = 'In this section';
        const fragment = document.createDocumentFragment();
        headings.forEach((heading) => {
            const link = document.createElement('a');
            link.href = `#${heading.id}`;
            link.className = 'page-toc-link';
            link.textContent = heading.textContent.trim();
            fragment.append(link);
        });
        tocNav.append(fragment);

        const links = [...tocNav.querySelectorAll('.page-toc-link')];
        headingObserver = new IntersectionObserver(
            (entries) => {
                const visible = entries.filter((entry) => entry.isIntersecting).sort((a, b) => a.boundingClientRect.top - b.boundingClientRect.top);
                if (!visible.length) return;
                const href = `#${visible[0].target.id}`;
                links.forEach((link) => {
                    const on = link.getAttribute('href') === href;
                    link.classList.toggle('active', on);
                    if (on) link.setAttribute('aria-current', 'location');
                    else link.removeAttribute('aria-current');
                });
            },
            { rootMargin: '-20% 0px -65% 0px', threshold: 0 }
        );
        headings.forEach((heading) => headingObserver.observe(heading));
    }

    const sectionObserver = new IntersectionObserver(
        (entries) => {
            const visible = entries.filter((entry) => entry.isIntersecting).sort((a, b) => a.boundingClientRect.top - b.boundingClientRect.top);
            if (!visible.length) return;
            if (tocNav.dataset.section === visible[0].target.id) return;
            tocNav.dataset.section = visible[0].target.id;
            render(visible[0].target);
        },
        { rootMargin: '-15% 0px -70% 0px', threshold: 0 }
    );

    sections.forEach((section) => sectionObserver.observe(section));
    render(sections[0]);
}

function initScrollProgress() {
    const bar = document.querySelector('.scroll-progress');
    if (!bar) return;

    const update = () => {
        const scrollTop = window.scrollY || document.documentElement.scrollTop;
        const scrollHeight = document.documentElement.scrollHeight - window.innerHeight;
        const progress = scrollHeight > 0 ? scrollTop / scrollHeight : 0;
        bar.style.transform = `scaleX(${Math.min(1, Math.max(0, progress))})`;
    };

    update();
    window.addEventListener('scroll', update, { passive: true });
    window.addEventListener('resize', update, { passive: true });
}

function debounce(fn, wait) {
    let timer;
    const wrapped = (...args) => {
        clearTimeout(timer);
        timer = setTimeout(() => fn(...args), wait);
    };
    wrapped.cancel = () => clearTimeout(timer);
    return wrapped;
}

function countLabel(count, singular, plural) {
    return `${count} ${count === 1 ? singular : plural}`;
}

function initFunctionFilter() {
    const input = document.getElementById('functionSearch');
    const root = document.getElementById('function-reference');
    const status = document.getElementById('function-search-status');
    if (!input || !root) return;

    const groups = [...root.querySelectorAll('.ref-group')];

    const announce = debounce((text) => {
        if (status) status.textContent = text;
    }, 200);

    input.addEventListener('input', () => {
        const query = input.value.trim().toLowerCase();
        let visibleCount = 0;
        groups.forEach((group) => {
            const articles = [...group.querySelectorAll('article.method')];
            if (!articles.length) {
                const show = query.length === 0 || group.textContent.toLowerCase().includes(query);
                group.hidden = !show;
                return;
            }
            let any = false;
            const heading = group.querySelector('h3')?.textContent.toLowerCase() || '';
            articles.forEach((article) => {
                const hide = query.length > 0 && !`${heading} ${article.textContent}`.toLowerCase().includes(query);
                article.hidden = hide;
                if (!hide) {
                    any = true;
                    visibleCount += 1;
                }
            });
            group.hidden = query.length > 0 && !any;
        });

        if (!query) {
            announce.cancel();
            if (status) status.textContent = '';
            return;
        }
        const text = visibleCount ? `${countLabel(visibleCount, 'function matches', 'functions match')}.` : 'No functions match that search.';
        announce(text);
    });
}

function initConstantFilter() {
    const input = document.getElementById('constantSearch');
    const root = document.getElementById('constant-reference');
    const status = document.getElementById('constant-search-status');
    if (!input || !root) return;

    const groups = [...root.querySelectorAll('.ref-group')];
    const rows = [];
    groups.forEach((group) => {
        const table = group.querySelector('table');
        if (!table) return;
        const heading = group.querySelector('h3')?.textContent.toLowerCase() || '';
        [...table.querySelectorAll('tbody tr')].forEach((row) => {
            rows.push({
                row,
                group,
                table,
                text: `${heading} ${row.textContent}`.toLowerCase()
            });
        });
    });

    const announce = debounce((text) => {
        if (status) status.textContent = text;
    }, 200);

    input.addEventListener('input', () => {
        const query = input.value.trim().toLowerCase();
        let visibleCount = 0;
        rows.forEach((item) => {
            const hide = query.length > 0 && !item.text.includes(query);
            item.row.hidden = hide;
            if (!hide) visibleCount += 1;
        });
        groups.forEach((group) => {
            const table = group.querySelector('table');
            const visible = [...(table?.querySelectorAll('tbody tr') || [])].some((row) => !row.hidden);
            group.hidden = query.length > 0 && !visible;
            if (table) table.hidden = query.length > 0 && !visible;
        });

        if (!query) {
            announce.cancel();
            if (status) status.textContent = '';
            return;
        }
        const text = visibleCount ? `${countLabel(visibleCount, 'constant matches', 'constants match')}.` : 'No constants match that search.';
        announce(text);
    });
}

document.addEventListener('DOMContentLoaded', () => {
    initTheme();
    initNavigation();
    initCodeBlocks();
    initSidebarSearch();
    handleResponsiveTables();
    initInstallTabs();
    initPageToc();
    initScrollProgress();
    initFunctionFilter();
    initConstantFilter();
});
